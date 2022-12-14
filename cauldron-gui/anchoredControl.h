#include "control.h"
#pragma once
namespace cauldron::gui {

	  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	 // A N C H O R E D   C O N T R O L - - - - - - - - - - - - - - - - - - - -
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class anchoredControl : public control {
	public:
		anchoredControl();
		
		//
		// Getters

		common::bounds2<f32> getAnchor() const;
		common::bounds2<f32> getOffset() const;
		common::bounds2<f32> getMargins() const;

		//
		// Setters

		virtual void setAnchor(const common::bounds2<f32>& new_anchor);
		virtual void setOffset(const common::bounds2<f32>& new_offset);
		virtual void setMargins(const common::bounds2<f32>& new_margins);

		void setBounds(const common::bounds2<i32>& new_bounds) override;

		void recalculateBounds();

		static common::bounds2<i32> calculateBounds(
			const common::bounds2<i32>& frame,
			const common::bounds2<f32>& anchor,
			const common::bounds2<f32>& offset);
		static common::bounds2<i32> boundsToOffset(
			const common::bounds2<i32>& frame,
			const common::bounds2<f32>& anchor,
			const common::bounds2<f32>& bounds);
	private:

		void recalcBoundsOnParentSize(control& sender, control::sizeData& e);
		void recalcBoundsOnParentSizing(control& sender, control::sizingData& e);
		void recalcBoundsOnParentChanged(control& sender, control::changeParentData& e);
		void onTerminateAnchored(control& sender, control::terminateData& e);

		common::bounds2<f32> _anchor;
		common::bounds2<i32> _offset;
	};
}