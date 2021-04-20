// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// An example of sending OpenCV webcam frames into a MediaPipe graph.
#include <cstdlib>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/classification.pb.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/Osc/OscSender.h"

constexpr char kInputStream[] = "input_video";
constexpr char kOutputStream[] = "output_video";
constexpr char kWindowName[] = "MediaPipe";
constexpr char kLandmarksStream[] = "landmarks";
constexpr char kHandidnessStream[] = "handedness";
constexpr char kMultipalmStream[] = "multi_palm_detections";

ABSL_FLAG(std::string, calculator_graph_config_file, "",
          "Name of file containing text format CalculatorGraphConfig proto.");
ABSL_FLAG(std::string, input_video_path, "",
          "Full path of video to load. "
          "If not provided, attempt to use a webcam.");
ABSL_FLAG(std::string, output_video_path, "",
          "Full path of where to save result (.mp4 only). "
          "If not provided, show result in a window.");

absl::Status RunMPPGraph() {
  OscSender sender;
  std::string calculator_graph_config_contents;
  MP_RETURN_IF_ERROR(mediapipe::file::GetContents(
      absl::GetFlag(FLAGS_calculator_graph_config_file),
      &calculator_graph_config_contents));
  LOG(INFO) << "Get calculator graph config contents: "
            << calculator_graph_config_contents;
  mediapipe::CalculatorGraphConfig config =
      mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
          calculator_graph_config_contents);

  LOG(INFO) << "Initialize the calculator graph.";
  mediapipe::CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  LOG(INFO) << "Initialize the camera or load the video.";
  cv::VideoCapture capture;
  const bool load_video = !absl::GetFlag(FLAGS_input_video_path).empty();
  if (load_video) {
    capture.open(absl::GetFlag(FLAGS_input_video_path));
  } else {
    capture.open(0);
  }
  RET_CHECK(capture.isOpened());

  cv::VideoWriter writer;
  const bool save_video = !absl::GetFlag(FLAGS_output_video_path).empty();
  if (!save_video) {
    cv::namedWindow(kWindowName, /*flags=WINDOW_AUTOSIZE*/ 1);
#if (CV_MAJOR_VERSION >= 3) && (CV_MINOR_VERSION >= 2)
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    capture.set(cv::CAP_PROP_FPS, 30);
#endif
  }

  LOG(INFO) << "Start running the calculator graph.";
  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
                   graph.AddOutputStreamPoller(kOutputStream));

  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller_landmarks,
                   graph.AddOutputStreamPoller(kLandmarksStream));

  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller_handidness,
                   graph.AddOutputStreamPoller(kHandidnessStream));

//    MP_RETURN_IF_ERROR(
//    graph.ObserveOutputStream(kLandmarksStream,
//                              [&graph](const mediapipe::Packet& packet) -> ::mediapipe::Status 
//                              {
//                                //std::cout << packet.DebugTypeName() << std::endl;
//                                //auto landmarks = packet.Get<std::vector<mediapipe::NormalizedLandmarkList>>();
//                                //std::cout << landmarks.size() << std::endl;
//                                //std::cout << landmarks.DebugString();
//                                //std::cout << "Handy!!!!" << std::endl;
//                                //int i = 0;
//                                //for (const auto& landmark : landmarks) 
//                                //    {
//                                //        std::cout << "*landmark*" << i++ << " : " << landmark.DebugString();
//                                //    }
//                                return mediapipe::OkStatus();
//                              }));
//
//    MP_RETURN_IF_ERROR(
//    graph.ObserveOutputStream(kHandidnessStream,
//                              [&graph](const mediapipe::Packet& packet) -> ::mediapipe::Status 
//                              {
//                                //std::cout << packet.DebugTypeName() << std::endl;
//                                auto handedness = packet.Get<std::vector<mediapipe::ClassificationList>>();
//                                //handidness.DebugString();
//                                //std::cout << "Handy!!!!" << std::endl;
//                                //int i = 0;
//                                //    for (const auto& hand : handedness) 
//                                //    {
//                                //        std::cout << "-hand-" << i++ << " : " << hand.DebugString();
//                                //    }
//                                return mediapipe::OkStatus();
//                              }));

    
//    MP_RETURN_IF_ERROR(
//    graph.ObserveOutputStream(kMultipalmStream,
//                              [&graph](const mediapipe::Packet& packet) -> ::mediapipe::Status 
//                              {
//                                std::cout << packet.DebugTypeName() << std::endl;
//                                //auto handedness = packet.Get<std::vector<mediapipe::ClassificationList>>();
//                                //handidness.DebugString();
//                                //std::cout << "Handy!!!!" << std::endl;
//                                //int i = 0;
//                                //    for (const auto& hand : handedness) 
//                                //    {
//                                //        std::cout << "-hand-" << i++ << " : " << hand.DebugString();
//                                //    }
//                                return mediapipe::OkStatus();
//                              }));

//      ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller_multipalm,
//                   graph.AddOutputStreamPoller(kMultipalmStream));

  MP_RETURN_IF_ERROR(graph.StartRun({}));

  LOG(INFO) << "Start grabbing and processing frames.";
  bool grab_frames = true;
  while (grab_frames) 
  {
    // Capture opencv camera or video frame.
    cv::Mat camera_frame_raw;
    capture >> camera_frame_raw;
    if (camera_frame_raw.empty()) 
    {
      if (!load_video) 
      {
        LOG(INFO) << "Ignore empty frames from camera.";
        continue;
      }
      LOG(INFO) << "Empty frame, end of video reached.";
      break;
    }
    cv::Mat camera_frame;
    cv::cvtColor(camera_frame_raw, camera_frame, cv::COLOR_BGR2RGB);
    if (!load_video) {
      cv::flip(camera_frame, camera_frame, /*flipcode=HORIZONTAL*/ 1);
    }

    // Wrap Mat into an ImageFrame.
    auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
        mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
        mediapipe::ImageFrame::kDefaultAlignmentBoundary);
    cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
    camera_frame.copyTo(input_frame_mat);

    // Send image packet into the graph.
    size_t frame_timestamp_us =
        (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
        kInputStream, mediapipe::Adopt(input_frame.release())
                          .At(mediapipe::Timestamp(frame_timestamp_us))));

    // Get the graph result packet, or stop if that fails.
    mediapipe::Packet packet;
    mediapipe::Packet landmark_packet;
    mediapipe::Packet handidness_packet;

    if (!poller.Next(&packet)) break;

    std::cout << poller_landmarks.QueueSize() << std::endl;
    
    if (poller_landmarks.QueueSize() != poller_handidness.QueueSize())
        std::cout << "Queue Missmatch:" << poller_landmarks.QueueSize() << " : " << poller_handidness.QueueSize() << std::endl;

    if (poller_landmarks.QueueSize() > 0 && poller_handidness.QueueSize() > 0)
    {
      if (!poller_handidness.Next(&handidness_packet)) break;
      auto& handidnessListVec = handidness_packet.Get<std::vector<::mediapipe::ClassificationList>>();

      if (!poller_landmarks.Next(&landmark_packet)) break;
      auto& landmarkListVec = landmark_packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();  

      if (handidnessListVec.size() == landmarkListVec.size())
      {
        for (int i = 0; i < handidnessListVec.size(); i++)
        {
          OscMessage mes;
          if (handidnessListVec[i].classification_size() > 1)
            std::cout << "classification_size greater than 1: " << handidnessListVec[i].classification_size() << std::endl;
          
          if (handidnessListVec[i].classification(0).index() == 0)
            mes.setAddressPattern ("/left");
          else if (handidnessListVec[i].classification(0).index() == 1)
            mes.setAddressPattern ("/right");

          for (int j = 0; j < landmarkListVec[i].landmark_size(); j++)
          {
            auto& landmark = landmarkListVec[i].landmark(j);
            mes.addFloat32 (landmark.x());
            mes.addFloat32 (landmark.y());
            mes.addFloat32 (landmark.z());
          }
          //
          //mes.addFloat32 (landmark.x());
          
          sender.send (mes, "127.0.0.1"   , 8000);
        }
      }
      else
      {
        std::cout << "LandmarkList is a different size to the handidnessList:" << std::endl;
      }
    // std::cout << "We have handidness" << std::endl;
    // auto& handidnessListVec = handidness_packet.Get<std::vector<::mediapipe::ClassificationList>>();
    // std::cout << "handidnessListVec_size: " << handidnessListVec.size() << std::endl;

    //for (auto& handidnessList : handidnessListVec)
    //{       
    //    //std::cout << handidnessList.DebugString() << std::endl;
    //    auto& classification = handidnessList.classification(0);
    //    std::cout << "dicpic: " << classification.index() << " : " << classification.label() << " : " << classification.score() <<  std::endl;
    //}

    // 
    // std::cout << "landmark_size: " << handidnessListVec.size() << std::endl;
    // 
    // std::cout << "We have landmarks" << std::endl;
    // auto& landmarkListVec = landmark_packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
    // for (auto& landmarkList : landmarkListVec)
    // {
    //   std::cout << "List size: " << landmarkList.landmark_size() << std::endl;
    //   std::cout << "First landmark: " << landmarkList.landmark(0).DebugString() << std::endl;
    //   //OscMessage message ("/tits", 1, 2.2f);
    //   //sender.send (message, "127.0.0.1"   , 8000);
    // }
      

    } 
    else 
    {
       std::cout << "Landmark and Handedness packect counts are different" << std::endl;
    }

 //   while (poller_multipalm.QueueSize() > 0)
 //     if (!poller_multipalm.Next(&multipalm_packet));

    auto& output_frame = packet.Get<mediapipe::ImageFrame>();
//    auto& output_landmarks = landmark_packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();

    // Convert back to opencv for display or saving.
    cv::Mat output_frame_mat = mediapipe::formats::MatView(&output_frame);
    cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
    if (save_video) {
      if (!writer.isOpened()) {
        LOG(INFO) << "Prepare video writer.";
        writer.open(absl::GetFlag(FLAGS_output_video_path),
                    mediapipe::fourcc('a', 'v', 'c', '1'),  // .mp4
                    capture.get(cv::CAP_PROP_FPS), output_frame_mat.size());
        RET_CHECK(writer.isOpened());
      }
      writer.write(output_frame_mat);
    } else {
      cv::imshow(kWindowName, output_frame_mat);
      // Press any key to exit.
      const int pressed_key = cv::waitKey(5);
      if (pressed_key >= 0 && pressed_key != 255) grab_frames = false;
    }

    // printout landmark values
//    for (const auto& landmark : output_landmarks) 
//    {
//        std::cout << landmark.DebugString();
//    }
  }

  LOG(INFO) << "Shutting down.";
  if (writer.isOpened()) writer.release();
  MP_RETURN_IF_ERROR(graph.CloseInputStream(kInputStream));
  return graph.WaitUntilDone();
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  absl::ParseCommandLine(argc, argv);
  absl::Status run_status = RunMPPGraph();
  if (!run_status.ok()) {
    LOG(ERROR) << "Failed to run the graph: " << run_status.message();
    return EXIT_FAILURE;
  } else {
    LOG(INFO) << "Success!";
  }
  return EXIT_SUCCESS;
}
