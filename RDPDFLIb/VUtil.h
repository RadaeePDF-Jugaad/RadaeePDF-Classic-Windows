#pragma once
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
namespace RDPDFLib
{
	namespace view
	{
		public interface class IVCanvas
		{
			virtual void onVPageChanged(int pageno) = 0;
			virtual void onVFound(bool found) = 0;
			virtual void onVCacheRendered(int pageno) = 0;
		};
		public interface class IVPanel
		{
			virtual void vpShowBlock(Image^ img) = 0;
			virtual void vpRemoveBlock(Image^ img) = 0;
		};
	}
}
