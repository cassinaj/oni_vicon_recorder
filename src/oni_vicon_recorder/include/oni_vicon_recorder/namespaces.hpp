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
 * @date 04/14/2014
 * @author Jan Issac (jan.issac@gmail.com)
 * Karlsruhe Institute of Technology (KIT), University of Southern California (USC)
 */

#ifndef ONI_VICON_RECORDER_NAMESPACES_HPP
#define ONI_VICON_RECORDER_NAMESPACES_HPP

/* define action and service namesspaces used by servers and clients */
// ViconRecorder
#define ACTION_NS_CONNECT_TO_VICON      "vicon_recorder/action/connect_to_vicon"
#define SERVICE_NS_VERIFY_OBJECT_EXISTS "vicon_recorder/srv/verify_object_exists"
#define SERVICE_NS_VICON_OBJECT_POSE    "vicon_recorder/srv/vicon_object_pose"
#define SERVICE_NS_VICON_OBJECTS        "vicon_recorder/srv/vicon_objects"

// OniViconRecorder
#define ACTION_NS_RECORD    "oni_vicon_recorder/action/record"

// OniRecorder
#define ACTION_NS_RUN_DEPTH_SENSOR          "oni_recorder/action/run_depth_sensor"
#define ACTION_NS_CHANGE_DEPTH_SENSOR_MODE  "oni_recorder/action/change_depth_sensor_mode"

#define ACTION_NS_GLOBAL_CALIBRATION  \
    "depth_sensor_vicon_calibration/action/global_calibration"
#define ACTION_NS_CONTINUE_GLOBAL_CALIBRATION  \
    "depth_sensor_vicon_calibration/action/continue_global_calibration"

#endif
