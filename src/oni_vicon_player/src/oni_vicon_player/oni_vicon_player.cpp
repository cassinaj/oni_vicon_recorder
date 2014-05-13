/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014 Max-Planck-Institute for Intelligent Systems,
 *                     University of Southern California,
 *                     Karlsruhe Institute of Technology
 *    Jan Issac (jan.issac@gmail.com)
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * @date 05/04/2014
 * @author Jan Issac (jan.issac@gmail.com)
 * Max-Planck-Institute for Intelligent Systems, University of Southern California (USC),
 *   Karlsruhe Institute of Technology (KIT)
 */

#include "oni_vicon_player/oni_vicon_player.hpp"

#include <boost/filesystem.hpp>

using namespace oni_vicon_player;
using namespace depth_sensor_vicon_calibration;

OniViconPlayer::OniViconPlayer(ros::NodeHandle& node_handle,
                               OniPlayer& oni_player,
                               ViconPlayer& vicon_player):
    oni_player_(oni_player),
    vicon_player_(vicon_player),
    open_as_(OpenGoal::ACTION_NAME,
             boost::bind(&OniViconPlayer::openCb, this, _1),
             false),
    play_as_(PlayGoal::ACTION_NAME,
             boost::bind(&OniViconPlayer::playCb, this, _1),
             false),
    open_(false),
    playing_(false),
    paused_(false)
{
    pause_srv_ = node_handle.advertiseService(Pause::Request::SERVICE_NAME,
                                              &OniViconPlayer::pauseCb,
                                              this);

    seek_frame_srv_ = node_handle.advertiseService(SeekFrame::Request::SERVICE_NAME,
                                                   &OniViconPlayer::seekFrameCb,
                                                   this);

    set_playback_speed_srv_ = node_handle.advertiseService(SetPlaybackSpeed::Request::SERVICE_NAME,
                                                           &OniViconPlayer::setPlaybackSpeedCb,
                                                           this);


}

OniViconPlayer::~OniViconPlayer()
{
}

void OniViconPlayer::run()
{
    open_as_.start();
    play_as_.start();

    ros::spin();
}

void OniViconPlayer::playCb(const PlayGoalConstPtr& goal)
{
    playing_ = true;

    PlayFeedback feedback;
    feedback.playing = true;
    feedback.current_time = 0;
    feedback.current_vicon_frame = 0;
    feedback.current_depth_sensor_frame = seeking_frame_;
    oni_player_.seekToFrame(seeking_frame_);

    play_as_.publishFeedback(feedback);

    while (ros::ok() && !play_as_.isPreemptRequested() && playing_)
    {
        if (paused_)
        {
            oni_player_.seekToFrame(seeking_frame_);
        }

        boost::mutex::scoped_lock lock(player_lock_);
        if (!oni_player_.process(ros::Time::now()))
        {
            break;
        }

        feedback.current_time = oni_player_.currentFrame()/30.;
        feedback.current_vicon_frame = 0;
        feedback.current_depth_sensor_frame = oni_player_.currentFrame();
        play_as_.publishFeedback(feedback);
    }

    paused_ = false;
    playing_ = false;
    oni_player_.seekToFrame(0);
    play_as_.setSucceeded();
}

void OniViconPlayer::openCb(const OpenGoalConstPtr& goal)
{
    OpenResult result;
    OpenFeedback feedback;
    feedback.progress_max = 100;

    boost::filesystem::path record_path = goal->record_path;
    std::string recording_name = record_path.leaf().string();

    std::string oni_file = (record_path / (recording_name + ".oni")).string();
    std::string vicon_file = (record_path / (recording_name + ".txt")).string();
    std::string global_calib_file = (record_path / "global_calibration.yaml").string();
    std::string local_calib_file = (record_path / "local_calibration.yaml").string();

    // Create calibration transform from calibration files
    CalibrationTransform calibration_transform;

    if (!calibration_transform.loadGlobalCalibration(global_calib_file))
    {
        result.message = "Loading global calibration file <" + global_calib_file + "> failed";
        open_as_.setAborted(result);
        return;
    }

    if (!calibration_transform.loadLocalCalibration(local_calib_file))
    {
        result.message = "Loading local calibration file <" + local_calib_file + "> failed";
        open_as_.setAborted(result);
        return;
    }

    // open player using the oni file and the camera intrinsics from the calibration
    if (!oni_player_.open(oni_file, calibration_transform.cameraIntrinsics()))
    {
        result.message = "Opening ONI file <" + oni_file + "> failed";
        open_as_.setAborted(result);
        oni_player_.close();
        return;
    }

    feedback.progress = 90;
    feedback.total_time = oni_player_.countFrames() / 30;
    feedback.total_vicon_frames = feedback.total_time * 100;
    feedback.total_depth_sensor_frames = oni_player_.countFrames();
    open_as_.publishFeedback(feedback);

    ros::Rate rate(30);
    while (ros::ok() && !open_as_.isPreemptRequested())
    {
        if (feedback.progress < feedback.progress_max)
        {
            ++feedback.progress;
            feedback.open = (feedback.progress == feedback.progress_max);
            open_as_.publishFeedback(feedback);
        }

        rate.sleep();
    }

    result.message = "Record closed";

    // stop player if playing
    paused_ = false;
    playing_ = false;
    // wait till oni player processing is over (if the controller is implemented correctly, this
    // should not be necessary)
    boost::mutex::scoped_lock lock(player_lock_);

    oni_player_.close();
    open_as_.setSucceeded(result);
}

bool OniViconPlayer::pauseCb(Pause::Request& request, Pause::Response& response)
{
    seeking_frame_ = oni_player_.currentFrame();
    paused_ = request.paused;

    return true;
}

bool OniViconPlayer::seekFrameCb(SeekFrame::Request& request, SeekFrame::Response &response)
{
    //boost::mutex::scoped_lock lock(player_lock_);
    seeking_frame_ = request.frame;
    return playing_ && paused_ && oni_player_.seekToFrame(request.frame);
}

bool OniViconPlayer::setPlaybackSpeedCb(SetPlaybackSpeed::Request& request,
                                        SetPlaybackSpeed::Response& response)
{
    return playing_ && paused_ && oni_player_.setPlaybackSpeed(request.speed);;
}
