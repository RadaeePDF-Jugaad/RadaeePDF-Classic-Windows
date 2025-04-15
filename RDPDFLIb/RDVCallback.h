#pragma once
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Core;
namespace RDPDFLib
{
	namespace view
	{
        public interface class IVCallback
        {
            virtual void vpDrawSelRect(double left, double top, double right, double bottom) = 0;
            virtual void vpDrawMarkRect(double left, double top, double right, double bottom) = 0;
            virtual void vpOnFound(bool found) = 0;
            virtual CoreDispatcher^ vpGetDisp() = 0;
            virtual void vpShowBlock(Image^ img, double x, double y, double w, double h) = 0;
            virtual void vpRemoveBlock(Image^ img) = 0;
            virtual void vpShowPNO(TextBlock^ txt, double left, double top, double right, double bottom) = 0;
            virtual void vpRemovePNO(TextBlock^ txt) = 0;
            virtual void vpDetachBmp(WriteableBitmap^ bmp) = 0;
            virtual void vpAttachBmp(WriteableBitmap^ bmp, const Array<byte>^ arr) = 0;
        };
    }
}
