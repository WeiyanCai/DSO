//
// Created by cwy on 2020/9/23.
//

#pragma once

#include <vector>
#include <memory>

namespace dso {
	class CoarseDistanceMap;

	class CalibHessian;

	class PointHessian;

	class FrameHessian;

	class PointFrameResidual;

	class PhotometricBA;

	class CuLocalWindow {
	public:
		using sint_type = int32_t;

		using calib_type = CalibHessian;

		using active_point_type = PointHessian;
		using active_frame_type = FrameHessian;
		using active_residual_type = PointFrameResidual;

		using active_frames_type = std::vector<active_frame_type*>;

		using ba_type = PhotometricBA;

//		using distance_map_type = CoarseDistanceMap;

	public:
		explicit CuLocalWindow(calib_type* calib);

		~CuLocalWindow();

	public:
		auto activeFramesNum() const -> sint_type;

		auto activePointsNum() const -> sint_type;

		auto residualsNum() const -> sint_type;

	public:
		auto activeFrames() -> active_frames_type;

		auto lastActiveFrame() -> active_frame_type*;

		void insertActiveFrame(active_frame_type* frame);

		void insertActivePoint(active_point_type* point);

		void insertActiveResidual(active_residual_type* residual);

	private:
		void precalculate();

	private:
		sint_type active_frames_num_;
		sint_type active_points_num_;
		sint_type residuals_num_;

		calib_type* calib_hessian_;

		active_frames_type active_frames_;

		std::unique_ptr<ba_type> ba_;

//		std::unique_ptr<distance_map_type> distance_map_;
	};
}
