#include "pch.h"
#include "PDFViewer.h"
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Shapes;
using namespace RDPDFLib::reader;
/*
float PDFViewer::sm_ovalWidth = 2;
unsigned int PDFViewer::sm_textColor = 0xFF000000;
unsigned int PDFViewer::sm_ovalColor = 0xFF0000FF;
float PDFViewer::sm_rectWidth = 2;
unsigned int PDFViewer::sm_rectColor = 0xFFFF0000;
float PDFViewer::sm_inkWidth = 3;
unsigned int PDFViewer::sm_inkColor = 0xFFFF0000;
float PDFViewer::sm_lineWidth = 2;
unsigned int PDFViewer::sm_lineColor = 0xFFFF0000;
PDF_LAYOUT_MODE PDFViewer::sm_viewMode = PDF_LAYOUT_MODE::layout_vert;
*/

void PDFVContent::vpShowBlock(Image^ img, double x, double y, double w, double h)
{
	if (w <= 0 || h <= 0) return;
	double mul_val = 1.0 / m_zoom;
	img->SetValue(Canvas::LeftProperty, x * mul_val);
	img->SetValue(Canvas::TopProperty, y * mul_val);
	img->Width = w * mul_val;
	img->Height = h * mul_val;
	Children->Append(img);
}

void PDFVContent::vpRemoveBlock(Image^ img)
{
	unsigned int idx = -1;
	if (Children->IndexOf(img, &idx))
		Children->RemoveAt(idx);
}

void PDFVContent::vpShowPNO(TextBlock ^txt, double left, double top, double right, double bottom)
{
	double ol = (double)txt->GetValue(Canvas::LeftProperty);
	double ot = (double)txt->GetValue(Canvas::TopProperty);
	double ow = txt->Width;
	double mul_val = 1.0 / m_zoom;
	double nl = left * mul_val;
	double nt = top + ((bottom - top) * mul_val - txt->FontSize * 1.5) * 0.5 * mul_val;
	double nw = (right - left) * mul_val;
	if (ol == nl && ot == nt && ow > nw - 0.00001 && ow < nw + 0.00001)//most happen when thumnail scrolling.
		return;
	txt->SetValue(Canvas::LeftProperty, nl);
	txt->SetValue(Canvas::TopProperty, nt);
	txt->Width = nw;
	if (txt->Parent)//most happen when thumbnail view resized.
	{
		unsigned int idx = 0;
		Children->IndexOf(txt, &idx);
		Children->RemoveAt(idx);
	}
	Children->Append(txt);
}

void PDFVContent::vpRemovePNO(TextBlock^ txt)
{
	unsigned int idx = -1;
	if (Children->IndexOf(txt, &idx))
		Children->RemoveAt(idx);
}
