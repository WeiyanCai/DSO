//
// Created by weiyancai on 2/11/20.
//

#include <thread>
//#include <locale.h>
#include <clocale>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "IOWrapper/Output3DWrapper.h"
#include "IOWrapper/ImageDisplay.h"


#include <boost/thread.hpp>
#include "util/settings.h"
#include "util/globalFuncs.h"
#include "util/DatasetReader.h"
#include "util/globalCalib.h"

#include "util/NumType.h"
#include "FullSystem/FullSystem.h"
#include "OptimizationBackend/MatrixAccumulators.h"
#include "FullSystem/PixelSelector2.h"


#include "IOWrapper/Pangolin/PangolinDSOViewer.h"
#include "IOWrapper/OutputWrapper/SampleOutputWrapper.h"

#include <celex5/celex5.h>
#include <celex5/celex5datamanager.h>
#include <celex5/celex5processeddata.h>

std::string vignette   = "";
std::string gammaCalib = "";
std::string source     = "";
std::string calib      = "";
std::string bin        = "";
std::string fpn        = "";
std::string saveFile   = "";

double rescale         = 1;
bool   reverse         = false;
bool   disableROS      = false;
int    start           = 0;
int    end             = 100000;
bool   prefetch        = false;
float  playbackSpeed
                       = 0;    // 0 for linearize (play as fast as possible, while sequentializing tracking & mapping). otherwise, factor on timestamps.
bool   preload         = false;
bool   useSampleOutput = false;

int mode = 0;

bool firstRosSpin = false;

void my_exit_handler(int s) {
	printf("Caught signal %d\n", s);
	exit(1);
}

void exitThread() {
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = my_exit_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	firstRosSpin = true;
	while (true) pause();
}

void settingsDefault(int preset) {
	printf("\n=============== PRESET Settings: ===============\n");

	if (preset == 0 || preset == 1) {
		printf("DEFAULT settings:\n"
		       "- %s real-time enforcing\n"
		       "- 2000 active points\n"
		       "- 5-7 active frames\n"
		       "- 1-6 LM iteration each KF\n"
		       "- original image resolution\n", preset == 0 ? "no " : "1x");

		playbackSpeed                  = (preset == 0 ? 0 : 1);
		preload                        = preset == 1;
		setting_desiredImmatureDensity = 1500;
		setting_desiredPointDensity    = 2000;
		setting_minFrames              = 5;
		setting_maxFrames              = 7;
		setting_maxOptIterations       = 6;
		setting_minOptIterations       = 1;
		setting_logStuff               = false;
	}

	if (preset == 2 || preset == 3) {
		printf("FAST settings:\n"
		       "- %s real-time enforcing\n"
		       "- 800 active points\n"
		       "- 4-6 active frames\n"
		       "- 1-4 LM iteration each KF\n"
		       "- 424 x 320 image resolution\n", preset == 0 ? "no " : "5x");

		playbackSpeed                  = (preset == 2 ? 0 : 5);
		preload                        = preset == 3;
		setting_desiredImmatureDensity = 600;
		setting_desiredPointDensity    = 800;
		setting_minFrames              = 4;
		setting_maxFrames              = 6;
		setting_maxOptIterations       = 4;
		setting_minOptIterations       = 1;

		benchmarkSetting_width  = 424;
		benchmarkSetting_height = 320;

		setting_logStuff = false;
	}

	printf("==============================================\n");
}

void parseArgument(char* arg) {
	int   option;
	float foption;
	char  buf[1000];

	Eigen::Vector2d a;
	auto b = a * a;

	if (1 == sscanf(arg, "sampleoutput=%d", &option)) {
		if (option == 1) {
			useSampleOutput = true;
			printf("USING SAMPLE OUTPUT WRAPPER!\n");
		}
		return;
	}

	if (1 == sscanf(arg, "quiet=%d", &option)) {
		if (option == 1) {
			setting_debugout_runquiet = true;
			printf("QUIET MODE, I'll shut up!\n");
		}
		return;
	}

	if (1 == sscanf(arg, "preset=%d", &option)) {
		settingsDefault(option);
		return;
	}

	if (1 == sscanf(arg, "rec=%d", &option)) {
		if (option == 0) {
			disableReconfigure = true;
			printf("DISABLE RECONFIGURE!\n");
		}
		return;
	}

	if (1 == sscanf(arg, "noros=%d", &option)) {
		if (option == 1) {
			disableROS         = true;
			disableReconfigure = true;
			printf("DISABLE ROS (AND RECONFIGURE)!\n");
		}
		return;
	}

	if (1 == sscanf(arg, "nolog=%d", &option)) {
		if (option == 1) {
			setting_logStuff = false;
			printf("DISABLE LOGGING!\n");
		}
		return;
	}

	if (1 == sscanf(arg, "reverse=%d", &option)) {
		if (option == 1) {
			reverse = true;
			printf("REVERSE!\n");
		}
		return;
	}

	if (1 == sscanf(arg, "nogui=%d", &option)) {
		if (option == 1) {
			disableAllDisplay = true;
			printf("NO GUI!\n");
		}
		return;
	}

	if (1 == sscanf(arg, "nomt=%d", &option)) {
		if (option == 1) {
			multiThreading = false;
			printf("NO MultiThreading!\n");
		}
		return;
	}

	if (1 == sscanf(arg, "prefetch=%d", &option)) {
		if (option == 1) {
			prefetch = true;
			printf("PREFETCH!\n");
		}
		return;
	}

	if (1 == sscanf(arg, "start=%d", &option)) {
		start = option;
		printf("START AT %d!\n", start);
		return;
	}

	if (1 == sscanf(arg, "end=%d", &option)) {
		end = option;
		printf("END AT %d!\n", start);
		return;
	}

	if (1 == sscanf(arg, "files=%s", buf)) {
		source = buf;
		printf("loading data from %s!\n", source.c_str());
		return;
	}

	if (1 == sscanf(arg, "calib=%s", buf)) {
		calib = buf;
		printf("loading calibration from %s!\n", calib.c_str());
		return;
	}

	if (1 == sscanf(arg, "vignette=%s", buf)) {
		vignette = buf;
		printf("loading vignette from %s!\n", vignette.c_str());
		return;
	}

	if (1 == sscanf(arg, "gamma=%s", buf)) {
		gammaCalib = buf;
		printf("loading gammaCalib from %s!\n", gammaCalib.c_str());
		return;
	}

	if (1 == sscanf(arg, "rescale=%f", &foption)) {
		rescale = foption;
		printf("RESCALE %f!\n", rescale);
		return;
	}

	if (1 == sscanf(arg, "speed=%f", &foption)) {
		playbackSpeed = foption;
		printf("PLAYBACK SPEED %f!\n", playbackSpeed);
		return;
	}

	if (1 == sscanf(arg, "save=%d", &option)) {
		if (option == 1) {
			debugSaveImages = true;
			if (42 == system("rm -rf images_out"))
				printf("system call returned 42 - what are the odds?. This is only here to shut up the compiler.\n");
			if (42 == system("mkdir images_out"))
				printf("system call returned 42 - what are the odds?. This is only here to shut up the compiler.\n");
			if (42 == system("rm -rf images_out"))
				printf("system call returned 42 - what are the odds?. This is only here to shut up the compiler.\n");
			if (42 == system("mkdir images_out"))
				printf("system call returned 42 - what are the odds?. This is only here to shut up the compiler.\n");
			printf("SAVE IMAGES!\n");
		}
		return;
	}

	if (1 == sscanf(arg, "mode=%d", &option)) {

		mode = option;
		if (option == 0) {
			printf("PHOTOMETRIC MODE WITH CALIBRATION!\n");
		}
		if (option == 1) {
			printf("PHOTOMETRIC MODE WITHOUT CALIBRATION!\n");
			setting_photometricCalibration = 0;
			setting_affineOptModeA         = 0; //-1: fix. >=0: optimize (with prior, if > 0).
			setting_affineOptModeB         = 0; //-1: fix. >=0: optimize (with prior, if > 0).
		}
		if (option == 2) {
			printf("PHOTOMETRIC MODE WITH PERFECT IMAGES!\n");
			setting_photometricCalibration = 0;
			setting_affineOptModeA         = -1; //-1: fix. >=0: optimize (with prior, if > 0).
			setting_affineOptModeB         = -1; //-1: fix. >=0: optimize (with prior, if > 0).
			setting_minGradHistAdd         = 3;
		}
		return;
	}

	if (1 == sscanf(arg, "bin=%s", buf)) {
		bin = buf;
		printf("loading bin file from %s!\n", bin.c_str());
		return;
	}

	if (1 == sscanf(arg, "fpn=%s", buf)) {
		fpn = buf;
		printf("loading fpn file from %s!\n", fpn.c_str());
		return;
	}

	if (1 == sscanf(arg, "savefile=%s", buf)) {
		saveFile = buf;
		printf("saving to %s on finish!\n", saveFile.c_str());
		return;
	}

	printf("could not parse argument \"%s\"!!!!\n", arg);
}

class CX5Observer : public CeleX5DataManager {
public:
	CX5Observer(CeleX5* sensor, CX5SensorDataServer* server, FullSystem* system, Undistort* undistorter)
			: sensor_ {sensor}, server_ {server}, system_ {system}, undistorter_ {undistorter}, frame_id_ {} {
		server_->registerData(this, CeleX5DataManager::CeleX_Frame_Data);
	}

//	CX5Observer(CeleX5* sensor, CX5SensorDataServer* server)
//			: sensor_ {sensor}, server_ {server}, frame_id_ {} {
//		server_->registerData(this, CeleX5DataManager::CeleX_Frame_Data);
//	}

	~CX5Observer() {
		server_->unregisterData(this, CeleX5DataManager::CeleX_Frame_Data);
	}

	virtual void onFrameDataUpdated(CeleX5ProcessedData* data);//overrides Observer operation

private:
	CeleX5             * sensor_;
	CX5SensorDataServer* server_;
	FullSystem         * system_;
	Undistort          * undistorter_;
	int frame_id_;
};

//uint8_t* pImageBuffer = new uint8_t[CELEX5_PIXELS_NUMBER];

void CX5Observer::onFrameDataUpdated(CeleX5ProcessedData* data) {
	if (!data) return;

	CeleX5::CeleX5Mode sensorMode = data->getSensorMode();

//	std::cout << "Receiving data: " << sensorMode << std::endl;

	if (CeleX5::Full_Picture_Mode == sensorMode) {
		std::cout << "receiving frame_id: " << frame_id_ << std::endl;

		auto buffer    = std::vector<uint8_t>(CELEX5_PIXELS_NUMBER);
		auto timestamp = std::time_t {};
		sensor_->getFullPicBuffer(buffer.data(), timestamp);

		auto img = cv::Mat(CELEX5_ROW, CELEX5_COL, CV_8UC1);
		std::copy_n(buffer.data(), CELEX5_PIXELS_NUMBER, img.data);

		MinimalImageB minImg((int) img.cols, (int) img.rows, (unsigned char*) img.data);
		ImageAndExposure* undistImg = undistorter_->undistort<unsigned char>(&minImg, 1, 0, 1.0f);
		undistImg->timestamp = static_cast<double>(timestamp); // relay the timestamp to dso
		system_->addActiveFrame(undistImg, frame_id_);
		frame_id_++;
		delete undistImg;
	}
//	} else if (CeleX5::Event_Off_Pixel_Timestamp_Mode == sensorMode) {
//		//get buffers when sensor works in EventMode
//		sensor_->getEventPicBuffer(pImageBuffer, CeleX5::EventBinaryPic);
//		cv::Mat matEventPic(800, 1280, CV_8UC1, pImageBuffer);
//		cv::imshow("Event Binary Pic", matEventPic);
//		cvWaitKey(1);
//
//	} else if (CeleX5::Optical_Flow_Mode == sensorMode) {
//		//full-frame optical-flow pic
//		sensor_->getOpticalFlowPicBuffer(pImageBuffer, CeleX5::OpticalFlowPic);
//		cv::Mat matOpticalFlow(800, 1280, CV_8UC1, pImageBuffer);
//		cv::imshow("Optical-Flow Pic", matOpticalFlow);
//		cvWaitKey(1);
//	}
}

int main(int argc, char** argv) {
	for (auto i = 1; i < argc; ++i) parseArgument(argv[i]);

	boost::thread exThread = boost::thread(exitThread);

	setting_desiredImmatureDensity = 1000;
	setting_desiredPointDensity    = 1200;
	setting_minFrames              = 5;
	setting_maxFrames              = 7;
	setting_maxOptIterations       = 4;
	setting_minOptIterations       = 1;
	setting_logStuff               = false;
	setting_kfGlobalWeight         = 1.3;

//	printf("MODE WITH CALIBRATION, but without exposure times!\n");
//	setting_photometricCalibration = 2;
//	setting_affineOptModeA         = 0;
//	setting_affineOptModeB         = 0;

	printf("PHOTOMETRIC MODE WITH PERFECT IMAGES!\n");
	setting_photometricCalibration = 0;
	setting_affineOptModeA = -1; //-1: fix. >=0: optimize (with prior, if > 0).
	setting_affineOptModeB = -1; //-1: fix. >=0: optimize (with prior, if > 0).
	setting_minGradHistAdd=3;

	auto* undistorter = dso::Undistort::getUndistorterForFile(calib, gammaCalib, vignette);

	setGlobalCalib(
			(int)undistorter->getSize()[0],
			(int)undistorter->getSize()[1],
			undistorter->getK().cast<float>());

	auto* fullSystem = new FullSystem;
	fullSystem->linearizeOperation = false;

//	disableAllDisplay = true;
	if (!disableAllDisplay) {
		fullSystem->outputWrapper.push_back(
				new IOWrap::PangolinDSOViewer(
						(int) undistorter->getSize()[0],
						(int) undistorter->getSize()[1]));
	}

	if (useSampleOutput) {
		fullSystem->outputWrapper.push_back(new IOWrap::SampleOutputWrapper());
	}

	if (undistorter->photometricUndist != nullptr) {
		fullSystem->setGammaFunction(undistorter->photometricUndist->getG());
	}

	auto* celex5 = new CeleX5;
	celex5->openSensor(CeleX5::CeleX5_MIPI);
	celex5->setFpnFile(fpn);

	if (!celex5->openBinFile(bin)) {
		printf("fail to open bin file %s", bin.c_str());
		exit(1);
	}

	auto* cx5_observer = new CX5Observer(celex5, celex5->getSensorDataServer(), fullSystem, undistorter);

	printf("start reading bin data...\n");

	while (!celex5->readBinFileData()) {
//		std::cout << "enter" << std::endl;
		usleep(1000);
	}

	printf("bin file has reached th end\n");

	fullSystem->printResult(saveFile);
	for (IOWrap::Output3DWrapper* ow : fullSystem->outputWrapper) {
		ow->join();
		delete ow;
	}

	delete undistorter;
	delete fullSystem;

	return 0;
};