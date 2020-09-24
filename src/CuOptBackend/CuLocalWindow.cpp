//
// Created by cwy on 2020/9/23.
//

#include "CuLocalWindow.h"

#include "CuPhotometricBA.h"
#include "CuParameterBlock.h"

#include "FullSystem/HessianBlocks.h"

namespace dso {
	CuLocalWindow::CuLocalWindow(CalibHessian* calib)
			: calib_hessian_ {calib}, active_frames_ {}, ba_(std::make_unique<ba_type>()) {}

	CuLocalWindow::~CuLocalWindow() = default;

	auto CuLocalWindow::activeFramesNum() const -> sint_type {
		return active_frames_num_;
	}

	auto CuLocalWindow::activePointsNum() const -> sint_type {
		return active_points_num_;
	}

	auto CuLocalWindow::residualsNum() const -> sint_type {
		return residuals_num_;
	}

	auto CuLocalWindow::activeFrames() -> active_frames_type {
		return active_frames_;
	}

	auto CuLocalWindow::lastActiveFrame() -> FrameHessian* {
		return active_frames_.back();
	}

	void CuLocalWindow::insertActiveFrame(FrameHessian* frame) {
		frame->idx = active_frames_.size();
		active_frames_.push_back(frame);

		ba_->frame_parameter_blocks().emplace_back(frame);

		// Prepare for linearize
		precalculate();
	}

	void CuLocalWindow::insertActivePoint(active_point_type* point) {
		ba_->point_parameter_blocks().emplace_back(point);
	}

	void CuLocalWindow::insertActiveResidual(active_residual_type* residual) {
		ba_->residual_blocks().emplace_back(residual);
	}

	void CuLocalWindow::precalculate() {
		for (auto* fh : active_frames_) {
			fh->targetPrecalc.resize(active_frames_.size());

			for (auto i = 0; i < active_frames_.size(); ++i) {
				fh->targetPrecalc[i].set(fh, active_frames_[i], calib_hessian_);
			}
		}
	}
}