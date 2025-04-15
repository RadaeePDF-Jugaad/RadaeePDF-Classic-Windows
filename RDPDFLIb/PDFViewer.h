#pragma once
#include "PDFCore.h"
#include "RDVLayout.h"
#include "RDVSel.h"
#include "RDVCallback.h"
#include <Windows.h>
using namespace RDPDFLib::pdf;
using namespace RDPDFLib::view;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Shapes;
using namespace Windows::UI::Input;

namespace RDPDFLib
{
	namespace reader
	{
		public interface class IPDFContentListener
		{
			void cFillRect(const PDFRect &rect, Windows::UI::Color clr);//to fill rectangles
			void cSetPos(double vx, double vy);//to set scroll position.
			void cAttachBmp(WriteableBitmap^ bmp, const Array<byte>^ arr);
			void cDetachBmp(WriteableBitmap^ bmp);
			void cFound(bool found);
		};
		public ref class PDFVCanvas sealed: public Canvas
		{
		public:
			PDFVCanvas() : Canvas()
			{
				IsHitTestVisible = false;
				Visibility = Windows::UI::Xaml::Visibility::Visible;
			}
			virtual ~PDFVCanvas()
			{
				clear();
			}
			/// <summary>
			/// Fill a PDFRect object with specified color
			/// </summary>
			/// <param name="rect">PDFRect object to fill</param>
			/// <param name="clr">Color to fill the rectangle</param>
			void fill_rect(PDFRect rect, Windows::UI::Color clr)
			{
				Rectangle^ rect1 = ref new Rectangle();
				rect1->SetValue(Canvas::LeftProperty, rect.left);
				rect1->SetValue(Canvas::TopProperty, rect.top);
				rect1->Width = (rect.right - rect.left);
				rect1->Height = (rect.bottom - rect.top);
				rect1->Fill = ref new SolidColorBrush(clr);
				Children->Append(rect1);
			}
			/// <summary>
			/// Draw a PDFRect to view
			/// </summary>
			/// <param name="rect">PDFRect object to draw</param>
			/// <param name="width">Line width of the rectangle</param>
			/// <param name="color">Line color of the rectangle</param>
			void draw_rect(PDFRect rect, float width, unsigned int color)
			{
				Rectangle^ rect1 = ref new Rectangle();
				Windows::UI::Color clr;
				clr.A = (byte)(color >> 24);
				clr.R = (byte)(color >> 16);
				clr.G = (byte)(color >> 8);
				clr.B = (byte)(color);
				rect1->Stroke = ref new SolidColorBrush(clr);
				rect1->StrokeThickness = width;
				rect1->SetValue(Canvas::LeftProperty, rect.left);
				rect1->SetValue(Canvas::TopProperty, rect.top);
				rect1->Width = rect.right - rect.left;
				rect1->Height = rect.bottom - rect.top;
				Children->Append(rect1);
			}
			/// <summary>
			/// Draw a PDFInk object to the view
			/// </summary>
			/// <param name="ink">PDFInk object to draw</param>
			/// <param name="path">Path data of the PDFInk object</param>
			/// <param name="pos"></param>
			/// <param name="offsetx"></param>
			/// <param name="offsety"></param>
			/// <returns></returns>
			int draw_ink(PDFInk^ ink, Path^ path, int pos, float offsetx, float offsety)
			{
				int cur = 0;
				int cnt = ink->NodesCnt;
				PDFPoint pt;
				PathGeometry^ inkg = (PathGeometry^)path->Data;
				PathFigure^ inkf = inkg->Figures->GetAt(inkg->Figures->Size - 1);
				LineSegment^ line;
				Windows::UI::Xaml::Media::BezierSegment^ bezier;
				Point ppt;
				for (cur = pos; cur < cnt; cur++)
				{
					switch (ink->GetOP(cur))
					{
					case 0:
						pt = ink->GetPoint(cur);
						ppt.X = pt.x + offsetx;
						ppt.Y = pt.y + offsety;
						inkf->StartPoint = ppt;
						break;
					case 1:
						pt = ink->GetPoint(cur);
						line = ref new LineSegment();
						ppt.X = pt.x + offsetx;
						ppt.Y = pt.y + offsety;
						line->Point = ppt;
						inkf->Segments->Append(line);
						break;
					case 2:
						pt = ink->GetPoint(cur);
						bezier = ref new Windows::UI::Xaml::Media::BezierSegment();
						ppt.X = pt.x + offsetx;
						ppt.Y = pt.y + offsety;
						bezier->Point1 = ppt;
						bezier->Point2 = ppt;
						pt = ink->GetPoint(cur + 1);
						ppt.X = pt.x + offsetx;
						ppt.Y = pt.y + offsety;
						bezier->Point3 = ppt;
						cur++;
						inkf->Segments->Append(bezier);
						break;
					}
				}
				Children->Append(path);
				return cnt;
			}
			/// <summary>
			/// Draw a polygon to the view
			/// </summary>
			/// <param name="polygon">A PDFPath which contains drawing operations</param>
			/// <param name="path">A Path object which is used stores the polygon data</param>
			/// <param name="pos">Index of operation which should be started from</param>
			/// <param name="offsetx">Offset on x coordinate</param>
			/// <param name="offsety">Offset on y coordinate</param>
			/// <returns>Node count of the ploygon</returns>
			int draw_polygon(PDFPath^ polygon, Path^ path, int pos, float offsetx, float offsety)
			{
				int cur = 0;
				int cnt = polygon->NodesCnt;
				PDFPoint pt;
				PathGeometry^ inkg = (PathGeometry^)path->Data;
				PathFigure^ inkf = inkg->Figures->GetAt(0);
				LineSegment^ line;
				Point ppt;
				Point ppts;
				Windows::Foundation::Size dotsz;
				dotsz.Width = 10;
				dotsz.Height = 10;
				for (cur = pos; cur < cnt; cur++)
				{
					switch (polygon->GetOP(cur))
					{
					case 0:
						pt = polygon->GetPoint(cur);
						ppt.X = pt.x + offsetx;
						ppt.Y = pt.y + offsety;
						inkf->StartPoint = ppt;
						break;
					case 1:
						pt = polygon->GetPoint(cur);
						line = ref new LineSegment();
						ppt.X = pt.x + offsetx;
						ppt.Y = pt.y + offsety;
						line->Point = ppt;
						inkf->Segments->Append(line);
						break;
					}
					PathFigure^ dotc = ref new PathFigure();
					ppts = ppt;
					ppts.X += 10;
					dotc->StartPoint = ppts;
					Windows::UI::Xaml::Media::ArcSegment^ dot = ref new Windows::UI::Xaml::Media::ArcSegment();
					dot->IsLargeArc = true;
					ppts.Y += 0.01;
					dot->Point = ppts;
					dot->Size = dotsz;
					//dot->SweepDirection = SweepDirection::Clockwise;
					dotc->Segments->Append(dot);
					dotc->IsClosed = true;
					inkg->Figures->Append(dotc);
				}
				Children->Append(path);
				return cnt;
			}
			/// <summary>
			/// Draw a polyline to the view
			/// </summary>
			/// <param name="polygon">A PDFPath which contains drawing operations</param>
			/// <param name="path">A Path object which is used stores the polyline data</param>
			/// <param name="pos">Index of operation which should be started from</param>
			/// <param name="offsetx">Offset on x coordinate</param>
			/// <param name="offsety">Offset on y coordinate</param>
			/// <returns>Node count of the ploygon</returns>
			int draw_polyline(PDFPath^ polygon, Path^ path, int pos, float offsetx, float offsety)
			{
				int cur = 0;
				int cnt = polygon->NodesCnt;
				PDFPoint pt;
				PathGeometry^ inkg = (PathGeometry^)path->Data;
				PathFigure^ inkf = inkg->Figures->GetAt(0);
				LineSegment^ line;
				Point ppt;
				Point ppts;
				Windows::Foundation::Size dotsz;
				dotsz.Width = 10;
				dotsz.Height = 10;
				for (cur = pos; cur < cnt; cur++)
				{
					switch (polygon->GetOP(cur))
					{
					case 0:
						pt = polygon->GetPoint(cur);
						ppt.X = pt.x + offsetx;
						ppt.Y = pt.y + offsety;
						inkf->StartPoint = ppt;
						break;
					case 1:
						pt = polygon->GetPoint(cur);
						line = ref new LineSegment();
						ppt.X = pt.x + offsetx;
						ppt.Y = pt.y + offsety;
						line->Point = ppt;
						inkf->Segments->Append(line);
						break;
					}
					PathFigure^ dotc = ref new PathFigure();
					ppts = ppt;
					ppts.X += 10;
					dotc->StartPoint = ppts;
					Windows::UI::Xaml::Media::ArcSegment^ dot = ref new Windows::UI::Xaml::Media::ArcSegment();
					dot->IsLargeArc = true;
					ppts.Y += 0.01;
					dot->Point = ppts;
					dot->Size = dotsz;
					dotc->Segments->Append(dot);
					dotc->IsClosed = true;
					inkg->Figures->Append(dotc);
				}
				Children->Append(path);
				return cnt;
			}
			/// <summary>
			/// Draw a stamp to the view
			/// </summary>
			/// <param name="pts">An array of points describes the position and size of the stamp</param>
			/// <param name="cnt">Count of points</param>
			/// <param name="bmp">A WriteableBitmap of the stamp image</param>
			/// <param name="offsetx">Offset on x coordinate</param>
			/// <param name="offsety">Offset on y coordinate</param>
			void draw_stamps(const Array <PDFPoint>^ pts, int cnt, WriteableBitmap ^bmp, float offsetx, float offsety)
			{
				int cur = 0;
				cnt <<= 1;
				for (cur = 0; cur < cnt; cur += 2)
				{
					PDFPoint pt0 = pts[cur];
					PDFPoint pt1 = pts[cur + 1];
					Image^ rect = ref new Image();
					if (pt0.x > pt1.x)
					{
						rect->SetValue(Canvas::LeftProperty, pt1.x + offsetx);
						rect->Width = pt0.x - pt1.x;
					}
					else
					{
						rect->SetValue(Canvas::LeftProperty, pt0.x + offsetx);
						rect->Width = pt1.x - pt0.x;
					}
					if (pt0.y > pt1.y)
					{
						rect->SetValue(Canvas::TopProperty, pt1.y + offsety);
						rect->Height = pt0.y - pt1.y;
					}
					else
					{
						rect->SetValue(Canvas::TopProperty, pt0.y + offsety);
						rect->Height = pt1.y - pt0.y;
					}
					rect->Stretch = Stretch::Fill;
					rect->Source = bmp;
					Children->Append(rect);
				}
			}
			/// <summary>
			/// Draw a Rectangle to the view
			/// </summary>
			/// <param name="pts">An array of points describes the position and size of the rectangle</param>
			/// <param name="cnt">Count of points</param>
			/// <param name="width">Line width of the rectangle</param>
			/// <param name="color">Color of the line</param>
			/// <param name="offsetx">Offset on x coordinate</param>
			/// <param name="offsety">Offset on y coordinate</param>
			void draw_rects(const Array <PDFPoint> ^pts, int cnt, float width, unsigned int color, float offsetx, float offsety)
			{
				Windows::UI::Color clr;
				clr.A = (byte)(color >> 24);
				clr.R = (byte)(color >> 16);
				clr.G = (byte)(color >> 8);
				clr.B = (byte)(color);
				SolidColorBrush^ br = ref new SolidColorBrush(clr);
				int cur = 0;
				cnt <<= 1;
				for (cur = 0; cur < cnt; cur += 2)
				{
					PDFPoint pt0 = pts[cur];
					PDFPoint pt1 = pts[cur + 1];
					Rectangle^ rect = ref new Rectangle();
					rect->StrokeThickness = width;
					rect->Stroke = br;
					if (pt0.x > pt1.x)
					{
						rect->SetValue(Canvas::LeftProperty, pt1.x + offsetx);
						rect->Width = pt0.x - pt1.x;
					}
					else
					{
						rect->SetValue(Canvas::LeftProperty, pt0.x + offsetx);
						rect->Width = pt1.x - pt0.x;
					}
					if (pt0.y > pt1.y)
					{
						rect->SetValue(Canvas::TopProperty, pt1.y + offsety);
						rect->Height = pt0.y - pt1.y;
					}
					else
					{
						rect->SetValue(Canvas::TopProperty, pt0.y + offsety);
						rect->Height = pt1.y - pt0.y;
					}
					Children->Append(rect);
				}
			}
			/// <summary>
			/// Draw a ellipse to the view
			/// </summary>
			/// <param name="pts">An array of points describes the position and size of the ellipse</param>
			/// <param name="cnt">Count of points</param>
			/// <param name="width">Line width of the rectangle</param>
			/// <param name="color">Color of the line</param>
			/// <param name="offsetx">Offset on x coordinate</param>
			/// <param name="offsety">Offset on y coordinate</param>
			void draw_ovals(const Array <PDFPoint> ^pts, int cnt, float width, unsigned int color, float offsetx, float offsety)
			{
				Windows::UI::Color clr;
				clr.A = (byte)(color >> 24);
				clr.R = (byte)(color >> 16);
				clr.G = (byte)(color >> 8);
				clr.B = (byte)(color);
				SolidColorBrush^ br = ref new SolidColorBrush(clr);
				int cur = 0;
				cnt <<= 1;
				float w = width * 0.5f;
				for (cur = 0; cur < cnt; cur += 2)
				{
					PDFPoint pt0 = pts[cur];
					PDFPoint pt1 = pts[cur + 1];
					Windows::UI::Xaml::Shapes::Ellipse^ rect = ref new Windows::UI::Xaml::Shapes::Ellipse();
					rect->StrokeThickness = width;
					rect->Stroke = br;
					if (pt0.x > pt1.x)
					{
						rect->SetValue(Canvas::LeftProperty, pt1.x - w + offsetx);
						rect->Width = pt0.x - pt1.x + width;
					}
					else
					{
						rect->SetValue(Canvas::LeftProperty, pt0.x - w + offsetx);
						rect->Width = pt1.x - pt0.x + width;
					}
					if (pt0.y > pt1.y)
					{
						rect->SetValue(Canvas::TopProperty, pt1.y - w + offsety);
						rect->Height = pt0.y - pt1.y + width;
					}
					else
					{
						rect->SetValue(Canvas::TopProperty, pt0.y - w + offsety);
						rect->Height = pt1.y - pt0.y + width;
					}
					Children->Append(rect);
				}
			}
			/// <summary>
			/// Draw lines to the view
			/// </summary>
			/// <param name="pts">An array of PDFPoint describes the lines</param>
			/// <param name="cnt">Count the points</param>
			/// <param name="width">Line width</param>
			/// <param name="color">Line color</param>
			/// <param name="offsetx">Offset on x coordinate</param>
			/// <param name="offsety">Offset on y coordinate</param>
			void draw_lines(const Array<PDFPoint> ^pts, int cnt, float width, unsigned int color, float offsetx, float offsety)
			{
				Windows::UI::Color clr;
				clr.A = (byte)(color >> 24);
				clr.R = (byte)(color >> 16);
				clr.G = (byte)(color >> 8);
				clr.B = (byte)(color);
				SolidColorBrush^ br = ref new SolidColorBrush(clr);
				int cur = 0;
				cnt <<= 1;
				float w = width * 0.5f;
				for (cur = 0; cur < cnt; cur += 2)
				{
					PDFPoint pt0 = pts[cur];
					PDFPoint pt1 = pts[cur + 1];
					Line^ line = ref new Line();
					line->StrokeThickness = width;
					line->Stroke = br;
					line->SetValue(Line::X1Property, pt0.x + offsetx);
					line->SetValue(Line::X2Property, pt1.x + offsetx);
					line->SetValue(Line::Y1Property, pt0.y + offsety);
					line->SetValue(Line::Y2Property, pt1.y + offsety);
					Children->Append(line);
				}
			}
			/// <summary>
			/// Draw lines and blocks to the view
			/// </summary>
			/// <param name="pts">An array of PDFPoint describes the position and size of lines and blocks</param>
			/// <param name="cnt">Count of the points</param>
			/// <param name="width">Line/Border width</param>
			/// <param name="gap">Gap and size of the blocks</param>
			/// <param name="color">Color for the lines and filling the blocks</param>
			void draw_lines_and_blks(const Array<PDFPoint>^ pts, int cnt, float width, int gap, unsigned int color)
			{
				Windows::UI::Color clr;
				clr.A = (byte)(color >> 24);
				clr.R = (byte)(color >> 16);
				clr.G = (byte)(color >> 8);
				clr.B = (byte)(color);
				SolidColorBrush^ br = ref new SolidColorBrush(clr);
				cnt--;
				for (int cur = 0; cur < cnt; cur++)
				{
					PDFPoint pt0 = pts[cur];
					PDFPoint pt1 = pts[cur + 1];
					Line^ line = ref new Line();
					line->StrokeThickness = width;
					line->Stroke = br;
					line->SetValue(Line::X1Property, pt0.x);
					line->SetValue(Line::X2Property, pt1.x);
					line->SetValue(Line::Y1Property, pt0.y);
					line->SetValue(Line::Y2Property, pt1.y);
					Children->Append(line);
				}
				clr.A = 255;
				clr.R = 0;
				clr.G = 0;
				clr.B = 0;
				SolidColorBrush^ brb = ref new SolidColorBrush(clr);
				clr.R = 255;
				clr.G = 255;
				clr.B = 255;
				SolidColorBrush^ brf = ref new SolidColorBrush(clr);
				cnt++;
				for (int cur = 0; cur < cnt; cur++)
				{
					PDFPoint pt0 = pts[cur];
					Rectangle^ rect1 = ref new Rectangle();
					rect1->Stroke = brb;
					rect1->Fill = brf;
					rect1->StrokeThickness = width;
					rect1->SetValue(Canvas::LeftProperty, pt0.x - gap);
					rect1->Width = (gap << 1);
					rect1->SetValue(Canvas::TopProperty, pt0.y - gap);
					rect1->Height = (gap << 1);
					Children->Append(rect1);
				}
			}
			void clear()
			{
				Children->Clear();
			}
		private:
		internal:
			void draw_rects(PDFPoint* pts, int cnt, float width, unsigned int color, float offsetx, float offsety)
			{
				Windows::UI::Color clr;
				clr.A = (byte)(color >> 24);
				clr.R = (byte)(color >> 16);
				clr.G = (byte)(color >> 8);
				clr.B = (byte)(color);
				SolidColorBrush^ br = ref new SolidColorBrush(clr);
				int cur = 0;
				cnt <<= 1;
				for (cur = 0; cur < cnt; cur += 2)
				{
					PDFPoint pt0 = pts[cur];
					PDFPoint pt1 = pts[cur + 1];
					Rectangle^ rect = ref new Rectangle();
					rect->StrokeThickness = width;
					rect->Stroke = br;
					if (pt0.x > pt1.x)
					{
						rect->SetValue(Canvas::LeftProperty, pt1.x + offsetx);
						rect->Width = pt0.x - pt1.x;
					}
					else
					{
						rect->SetValue(Canvas::LeftProperty, pt0.x + offsetx);
						rect->Width = pt1.x - pt0.x;
					}
					if (pt0.y > pt1.y)
					{
						rect->SetValue(Canvas::TopProperty, pt1.y + offsety);
						rect->Height = pt0.y - pt1.y;
					}
					else
					{
						rect->SetValue(Canvas::TopProperty, pt0.y + offsety);
						rect->Height = pt1.y - pt0.y;
					}
					Children->Append(rect);
				}
			}
			void draw_ovals(PDFPoint* pts, int cnt, float width, unsigned int color, float offsetx, float offsety)
			{
				Windows::UI::Color clr;
				clr.A = (byte)(color >> 24);
				clr.R = (byte)(color >> 16);
				clr.G = (byte)(color >> 8);
				clr.B = (byte)(color);
				SolidColorBrush^ br = ref new SolidColorBrush(clr);
				int cur = 0;
				cnt <<= 1;
				float w = width * 0.5f;
				for (cur = 0; cur < cnt; cur += 2)
				{
					PDFPoint pt0 = pts[cur];
					PDFPoint pt1 = pts[cur + 1];
					Windows::UI::Xaml::Shapes::Ellipse^ rect = ref new Windows::UI::Xaml::Shapes::Ellipse();
					rect->StrokeThickness = width;
					rect->Stroke = br;
					if (pt0.x > pt1.x)
					{
						rect->SetValue(Canvas::LeftProperty, pt1.x - w + offsetx);
						rect->Width = pt0.x - pt1.x + width;
					}
					else
					{
						rect->SetValue(Canvas::LeftProperty, pt0.x - w + offsetx);
						rect->Width = pt1.x - pt0.x + width;
					}
					if (pt0.y > pt1.y)
					{
						rect->SetValue(Canvas::TopProperty, pt1.y - w + offsety);
						rect->Height = pt0.y - pt1.y + width;
					}
					else
					{
						rect->SetValue(Canvas::TopProperty, pt0.y - w + offsety);
						rect->Height = pt1.y - pt0.y + width;
					}
					Children->Append(rect);
				}
			}
			void draw_lines(PDFPoint* pts, int cnt, float width, unsigned int color, float offsetx, float offsety)
			{
				Windows::UI::Color clr;
				clr.A = (byte)(color >> 24);
				clr.R = (byte)(color >> 16);
				clr.G = (byte)(color >> 8);
				clr.B = (byte)(color);
				SolidColorBrush^ br = ref new SolidColorBrush(clr);
				int cur = 0;
				cnt <<= 1;
				float w = width * 0.5f;
				for (cur = 0; cur < cnt; cur += 2)
				{
					PDFPoint pt0 = pts[cur];
					PDFPoint pt1 = pts[cur + 1];
					Line^ line = ref new Line();
					line->StrokeThickness = width;
					line->Stroke = br;
					line->SetValue(Line::X1Property, pt0.x + offsetx);
					line->SetValue(Line::X2Property, pt1.x + offsetx);
					line->SetValue(Line::Y1Property, pt0.y + offsety);
					line->SetValue(Line::Y2Property, pt1.y + offsety);
					Children->Append(line);
				}
			}
		};

		public interface class IPDFViewListener
		{
			void OnPDFPageUpdated(int pageno);
			void OnPDFScaleChanged(double scale);
			/// <summary>
			/// fired when page no changed.
			/// </summary>
			/// <param name="pageno">page NO.</param>
			void OnPDFPageChanged(int pageno);
			/// <summary>
			/// fired when single tapped on page, without annotations.
			/// </summary>
			/// <param name="x"></param>
			/// <param name="y"></param>
			/// <returns></returns>
			void OnPDFSingleTapped(float x, float y);
			void OnPDFPageTapped(int pageno);
			void OnPDFLongPressed(float x, float y);
			/// <summary>
			/// fired when text found.
			/// </summary>
			/// <param name="found">true if found, false if end of PDF.</param>
			void OnPDFFound(Boolean found);
			/// <summary>
			/// fired when selecting.
			/// </summary>
			/// <param name="canvas">Canvas object to draw.</param>
			/// <param name="rect1">first char's location, in Canvas coordinate.</param>
			/// <param name="rect2">last char's location, in Canvas coordinate.</param>
			void OnPDFSelecting(Canvas^ canvas, PDFRect rect1, PDFRect rect2);
			/// <summary>
			/// fired when text selected.
			/// using vGetSelText to get texts.
			/// using vSelEnd to end selecting status.
			/// using vSelMarkup to set markup annotation.
			/// </summary>
			void OnPDFSelected();
			/// <summary>
			/// fired when an annotation single tapped.
			/// this means enter into annotation status.
			/// </summary>
			/// <param name="page">page object annotation included.</param>
			/// <param name="annot">clicked annotation.</param>
			void OnPDFAnnotClicked(PDFPage^ page, int pageno, PDFAnnot^ annot, PDFRect rect);
			/// <summary>
			/// fired when annotation status leaved.
			/// </summary>
			void OnPDFAnnotEnd();

			/// <summary>
			/// fired when goto link annotation performed.
			/// </summary>
			/// <param name="pageno"></param>
			void OnPDFAnnotGoto(int pageno);
			/// <summary>
			/// fired when uri link annotation performed.
			/// </summary>
			/// <param name="uri">uri address.</param>
			void OnPDFAnnotURI(String^ uri);
			///// <summary>
			///// fired when a movie annotation performed.
			///// </summary>
			///// <param name="annot">Annotation object</param>
			///// <param name="name">file name of movie without path.</param>
			//void OnPDFAnnotMovieAsync(PDFAnnot annot, String name);
			///// <summary>
			///// fired when a sound annotation performed.
			///// </summary>
			///// <param name="annot">Annotation object</param>
			///// <param name="name">file name of sound without path.</param>
			//void OnPDFAnnotSound(PDFAnnot annot, String name);
			/// <summary>
			/// fired when an annotation with rich media performed
			/// </summary>
			/// <param name="annot">Annotation object</param>
			void OnPDFAnnotRichMedia(PDFAnnot^ annot);
			void OnPDFAnnotRendition(PDFAnnot^ annot);

			/// <summary>
			/// fired when vAnnotPerform invoked or Note Annotation added.
			/// </summary>
			/// <param name="annot">Annotation object</param>
			/// <param name="subj">subject</param>
			/// <param name="text">text in popup window</param>
			void OnPDFAnnotPopup(PDFAnnot^ annot, String^ subj, String^ text);

			/// <summary>
			/// fired when remote destination (external file) annotation performed.
			/// </summary>
			/// <param name="dest">remote dest (the external file name with optional page number).</param>
			void OnPDFAnnotRemoteDest(String^ dest);
			/// <summary>
			/// fired when file link annotation performed.
			/// </summary>
			/// <param name="filelink">file link annotation path.</param>
			void OnPDFAnnotFileLink(String^ filelink);
			//void OnPDFZoomStart();
			//void OnPDFZoomEnd();
		};
		public enum class PDF_LAYOUT_MODE
		{
			layout_unknown = 0,
			layout_vert,
			layout_horz,
			layout_dual,
			layout_dual_cover,
			layout_hsingle,
			layout_hdual,
		};
		//all coordinate transform done in this class.
		public ref class PDFVContent sealed : public Canvas, IVCallback
		{
		public:
			PDFVContent() :Canvas()
			{
				Windows::Graphics::Display::DisplayInformation^ disp = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
				m_doc = nullptr;
				m_layout = NULL;
				m_zoom = 1;
				m_bound = nullptr;
				m_page_sel = -1;
			}
			virtual ~PDFVContent()
			{
				vClose();
			}
			/// <summary>
			/// Get scale of the content
			/// </summary>
			/// <returns>The scale</returns>
			inline double vGetScale()
			{
				if (m_layout) return m_layout->vGetScale();
				else return -1;
			}
			/// <summary>
			/// Get PDFPos base on specified x and y coordinates on view element.
			/// </summary>
			/// <param name="vx">x coordinate on view element</param>
			/// <param name="vy">y coordinate on view element</param>
			/// <returns></returns>
			inline PDFPos vGetPos(double vx, double vy)
			{
				if (m_layout) return m_layout->vGetPos(vx, vy);
				PDFPos pos;
				pos.pageno = -1;
				pos.x = 0;
				pos.y = 0;
				return pos;
			}
			/// <summary>
			/// Check if it is possible to set position of the content 
			/// </summary>
			/// <returns>True or false</returns>
			inline bool vCanSetPos()
			{
				return (m_layout->vGetLayW() > 0 && m_layout->vGetLayH() > 0);
			}
			/// <summary>
			/// Set and jump to specified position
			/// </summary>
			/// <param name="vx">offset on x coordinate on view component</param>
			/// <param name="vy">offset on y coordinate on view component</param>
			/// <param name="pos">A PDFPos object to specify where the content should be placed on the PDF document</param>
			inline void vSetPos(double vx, double vy, const PDFPos& pos)
			{
				if (m_layout)
				{
					m_layout->vSetPos(vx, vy, pos);
					m_callback->cSetPos(m_layout->vGetX(), m_layout->vGetY());
				}
			}
			/// <summary>
			/// Get view x of the content
			/// </summary>
			/// <returns>x coordinate on view content</returns>
			inline double vGetX()
			{
				if (m_layout) return m_layout->vGetX();
				return 0;
			}
			/// <summary>
			/// Get view y of the content
			/// </summary>
			/// <returns>y coordinate on view content</returns>
			inline double vGetY()
			{
				if (m_layout) return m_layout->vGetY();
				return 0;
			}
			/// <summary>
			/// Set selected page
			/// </summary>
			/// <param name="pageno">0 base page index</param>
			inline void vSetPageSel(int pageno)
			{
				if (!m_layout || !m_doc || m_page_sel == pageno || pageno < 0)
					return;
				m_page_sel = pageno;
				CRDVPage* vpage = m_layout->vGetPage(pageno);
				PDFPos pos;
				pos.pageno = m_page_sel;
				pos.x = 0.5 * m_doc->GetPageWidth(m_page_sel);
				pos.y = 0.5 * m_doc->GetPageHeight(m_page_sel);
				m_layout->vSetPos(m_layout->vGetVW() * 0.5, m_layout->vGetVH() * 0.5, pos);
				if (m_callback)
				{
					m_callback->cSetPos(m_layout->vGetX(), m_layout->vGetY());
					PDFRect rect;
					rect.left = vpage->GetLeft();
					rect.top = vpage->GetTop();
					rect.right = vpage->GetRight();
					rect.bottom = vpage->GetBottom();
					Windows::UI::Color clr;
					clr.A = 64;
					clr.B = 224;
					clr.G = 0;
					clr.R = 0;
					m_callback->cFillRect(rect, clr);
				}
			}
			/// <summary>
			/// Get index of selected page
			/// </summary>
			/// <returns>Index of selected page</returns>
			inline int vGetPageSel()
			{
				return m_page_sel;
			}
			/// <summary>
			/// Get a reference of a PDF view page.
			/// </summary>
			/// <param name="pgno"></param>
			/// <returns>Reference of page</returns>
			inline long long vGetPage(int pgno)
			{
				if (m_layout) return (long long)m_layout->vGetPage(pgno);
				return NULL;
			}
			/// <summary>
			/// Get a reference of a PDF view page with specified coordinate
			/// </summary>
			/// <param name="vx">x coordinate on view component</param>
			/// <param name="vy">y coordinate on view component</param>
			/// <returns>Reference of page</returns>
			inline int vGetPage(double vx, double vy)
			{
				if (m_layout) return m_layout->vGetPage(vx, vy);
				return -1;
			}
			/// <summary>
			/// Open and display a opened PDFDoc
			/// </summary>
			/// <param name="doc">Opened PDFDoc to display on the view</param>
			/// <param name="lmode">View mode for displaying, please refer to definition of PDF_LAYOUT_MODE</param>
			/// <param name="callback">Callcall to deal with graphic tasks. Please refer to definition of IPDFContentListener</param>
			/// <returns>True if successed, otherwise false</returns>
			inline bool vOpen(PDFDoc^ doc, PDF_LAYOUT_MODE lmode, IPDFContentListener ^callback)
			{
				if (!doc) return false;
				m_doc = doc;
				m_callback = callback;
				vSetView(lmode);
				m_page_sel = -1;
				return true;
			}
			/// <summary>
			/// Set and update view mode
			/// </summary>
			/// <param name="vmode">New view mode, please refer to definition of PDF_LAYOUT_MODE</param>
			inline void vSetView(PDF_LAYOUT_MODE vmode)
			{
				if (!m_doc) return;
				if (m_layout)
				{
					m_layout->vClose();
					m_layout = NULL;
				}
				switch (vmode)
				{
				case PDF_LAYOUT_MODE::layout_vert:
					m_layout = new CRDVLayoutVert(CRDVLayoutVert::ALIGN_CENTER, false);
					break;
				case PDF_LAYOUT_MODE::layout_horz:
					m_layout = new CRDVLayoutHorz(CRDVLayoutHorz::ALIGN_CENTER, false);
					break;
				case PDF_LAYOUT_MODE::layout_dual:
					m_layout = new CRDVLayoutDual(CRDVLayoutDual::SCALE_NONE, false, false);
					break;
				case PDF_LAYOUT_MODE::layout_dual_cover:
					m_layout = new CRDVLayoutDual(CRDVLayoutDual::SCALE_NONE, false, true);
					break;
				case PDF_LAYOUT_MODE::layout_hsingle:
					m_layout = new CRDVLayoutDualH(CRDVLayoutDualH::SCALE_NONE, CRDVLayoutDualH::ALIGN_CENTER, false, NULL, 0, NULL, 0);
					break;
				case PDF_LAYOUT_MODE::layout_hdual:
				{
					bool* bhorz = (bool *)malloc(sizeof(bool) * m_doc->PageCount);
					memset(bhorz, 1, sizeof(bool) * m_doc->PageCount);
					m_layout = new CRDVLayoutDualH(CRDVLayoutDualH::SCALE_NONE, CRDVLayoutDualH::ALIGN_CENTER, false, NULL, 0, bhorz, m_doc->PageCount);
					free(bhorz);
				}
					break;
				case (PDF_LAYOUT_MODE)100:
					m_layout = new CRDVLayoutThumbH(CRDVLayoutThumbH::ALIGN_CENTER, false);
					break;
				case (PDF_LAYOUT_MODE)101:
					m_layout = new CRDVLayoutThumbV(CRDVLayoutThumbV::ALIGN_CENTER, false);
					break;
				default:
					m_layout = new CRDVLayoutVert(CRDVLayoutVert::ALIGN_CENTER, true);
					break;
				}
				m_layout->vOpen(m_doc, this, 4);
				InvalidateMeasure();
			}
			/// <summary>
			/// Set if auto resize the PDF pages to fit view size.
			/// </summary>
			/// <param name="autofit"></param>
			inline void vSetAutoFit(bool autofit)
			{
				if (!m_layout) return;
				m_layout->vSetAutoFit(autofit);
			}
			/// <summary>
			/// Check if auto fit view size is set
			/// </summary>
			/// <returns>True or false</returns>
			inline bool vGetAutoFit()
			{
				if (!m_layout) return false;
				return m_layout->vGetAutoFit();
			}
			/// <summary>
			/// Resize the view
			/// </summary>
			/// <param name="vw">New view width</param>
			/// <param name="vh">New view height</param>
			/// <param name="scale">PDF page scale</param>
			inline void vResize(double vw, double vh, double scale)
			{
				m_zoom = scale;
				if (m_layout)
				{
					m_layout->vResize(vw, vh);
					if(m_layout->vGetAutoFit())
						m_zoom = m_layout->vGetScale();//update zoom value to autofit value.
					if (m_page_sel >= 0)
					{
						int pgno = m_page_sel;
						m_page_sel = -1;
						vSetPageSel(pgno);
					}
					InvalidateMeasure();
				}
			}
			/// <summary>
			/// Draw on view
			/// </summary>
			/// <param name="vx">x coordinate to draw on view</param>
			/// <param name="vy">y coordinate to draw on view</param>
			inline void vDraw(double vx, double vy)
			{
				if (m_layout)
				{
					m_layout->vSetX(vx);
					m_layout->vSetY(vy);
					m_layout->vDraw();
				}
			}
			/// <summary>
			/// Draw highlight background mark the text found during search
			/// </summary>
			/// <param name="vx">x coordinate of the top left point of highlight area</param>
			/// <param name="vy">y coordinate of the top left point of highlight area</param>
			inline void vDrawFind(double vx, double vy)
			{
				if (m_layout)
					m_layout->vDrawFind(vx, vy);
			}
			/// <summary>
			/// Activate text search mode
			/// </summary>
			/// <param name="pat">Texts to search</param>
			/// <param name="match_case">True if search with case sensitive, otherwise false</param>
			/// <param name="whole_word">True if search with whole word match, otherwise false</param>
			inline void vFindStart(String^ pat, bool match_case, bool whole_word)
			{
				if (m_layout)
					m_layout->vFindStart(pat, match_case, whole_word);
			}
			/// <summary>
			/// Find next match to texts set in vFindStart
			/// </summary>
			/// <param name="dir">Direction to search. if dir < 0, search backward, otherwise search forward</param>
			inline void vFind(int dir)
			{
				if (m_layout)
					m_layout->vFind(dir);
			}
			/// <summary>
			/// Deactivate text search mode
			/// </summary>
			inline void vFindEnd()
			{
				if (m_layout)
					m_layout->vFindEnd();
			}
			/// <summary>
			/// Close view and release holding resources
			/// </summary>
			inline void vClose()
			{
				if (!m_doc) return;
				m_bound = nullptr;
				if (m_layout)
				{
					m_layout->vClose();
					//delete in delegate callback
					m_layout = NULL;
				}
				m_callback = nullptr;
				Children->Clear();
				m_doc = nullptr;
				m_page_sel = -1;
			}
			/// <summary>
			/// Activate zoom mode
			/// </summary>
			inline void vZoomStart()
			{
				if (!m_layout) return;
				m_layout->vZoomStart();
			}
			/// <summary>
			/// Deactivate zoom mode
			/// </summary>
			inline void vZoomConfirm()
			{
				if (!m_layout) return;
				m_layout->vZoomConfirm();
			}
			/// <summary>
			/// Set scale and zoom in/out current displaying content
			/// </summary>
			/// <param name="scale">zoom scale</param>
			inline void vZoomSet(double scale)
			{
				if (!m_layout) return;
				m_zoom = scale;
				m_layout->vZoomSet(scale);
			}
			/// <summary>
			/// Render a PDF page on view
			/// </summary>
			/// <param name="page">Reference of a PDF page to render</param>
			inline void vRenderPage(long long page)
			{
				if (!page) return;
				((CRDVPage *)page)->ui_set_dirty();
			}
			static inline int pgGetPageNo(long long vpage) { return ((CRDVPage*)vpage)->GetPageNo(); }
			static inline int pgGetLeft(long long vpage) { return ((CRDVPage*)vpage)->GetLeft(); }
			static inline int pgGetTop(long long vpage) { return ((CRDVPage*)vpage)->GetTop(); }
			static inline int pgGetRight(long long vpage) { return ((CRDVPage*)vpage)->GetRight(); }
			static inline int pgGetBottom(long long vpage) { return ((CRDVPage*)vpage)->GetBottom(); }
			static inline int pgGetWidth(long long vpage) { return ((CRDVPage*)vpage)->GetWidth(); }
			static inline int pgGetHeight(long long vpage) { return ((CRDVPage*)vpage)->GetHeight(); }
			static inline double pgGetScale(long long vpage) { return ((CRDVPage*)vpage)->GetScale(); }
			static inline double pgGetPDFX(long long vpage, double vx) { return ((CRDVPage*)vpage)->GetPDFX(vx); }
			static inline double pgGetPDFY(long long vpage, double vy) { return ((CRDVPage*)vpage)->GetPDFY(vy); }
			static inline double pgGetVX(long long vpage, double pdfx) { return ((CRDVPage*)vpage)->GetVX(pdfx); }
			static inline double pgGetVY(long long vpage, double pdfy) { return ((CRDVPage*)vpage)->GetVY(pdfy); }
			static inline double pgToDIBX(long long vpage, double x) { return ((CRDVPage*)vpage)->ToDIBX(x); }
			static inline double pgToDIBY(long long vpage, double y) { return ((CRDVPage*)vpage)->ToDIBY(y); }
			static inline PDFMatrix^ pgCreateInvertMatrix(long long vpage, double scrollx, double scrolly)
			{
				return ((CRDVPage*)vpage)->CreateInvertMatrix(scrollx, scrolly);
			}
		public:
			/// <summary>
			/// Draw text selecting rectangle to mark selected texts
			/// </summary>
			/// <param name="left">x coordinate of the top left point of the rectangle</param>
			/// <param name="top">y coordinate of the top left point of the rectangle</param>
			/// <param name="right">x coordinate of the bottom right point of the rectangle</param>
			/// <param name="bottom">y coordinate of the bottom right point of the rectangle</param>
			virtual void vpDrawSelRect(double left, double top, double right, double bottom)
			{
				if (!m_callback) return;
				PDFRect rect;
				rect.left = left;
				rect.top = top;
				rect.right = right;
				rect.bottom = bottom;
				Windows::UI::Color clr;
				clr.A = 64;
				clr.B = 255;
				clr.G = 0;
				clr.R = 0;
				m_callback->cFillRect(rect, clr);
			}
			/// <summary>
			/// Draw a mark up rectangle
			/// </summary>
			/// <param name="left">x coordinate of the top left point of the rectangle</param>
			/// <param name="top">y coordinate of the top left point of the rectangle</param>
			/// <param name="right">x coordinate of the bottom right point of the rectangle</param>
			/// <param name="bottom">y coordinate of the bottom right point of the rectangle</param>
			virtual void vpDrawMarkRect(double left, double top, double right, double bottom)
			{
				if (!m_callback) return;
				PDFRect rect;
				rect.left = left;
				rect.top = top;
				rect.right = right;
				rect.bottom = bottom;
				Windows::UI::Color clr;
				clr.A = 64;
				clr.B = 0;
				clr.G = 0;
				clr.R = 0;
				m_callback->cFillRect(rect, clr);
			}
			virtual void vpShowBlock(Image^ img, double x, double y, double w, double h);
			virtual void vpRemoveBlock(Image^ img);
			virtual void vpShowPNO(TextBlock^ txt, double left, double top, double right, double bottom);
			virtual void vpRemovePNO(TextBlock^ txt);
			/// <summary>
			/// callback for text search mode. Invoked on text found or search reaches the end of page
			/// </summary>
			/// <param name="found"></param>
			virtual void vpOnFound(bool found)
			{
				if (found)
				{
					m_layout->vFindGoto();
					if (m_callback)
					{
						m_callback->cSetPos(m_layout->vGetX(), m_layout->vGetY());
						m_callback->cFound(found);
					}
				}
				else if (m_callback)
					m_callback->cFound(found);
			}
			/// <summary>
			/// Detach specified image
			/// </summary>
			/// <param name="bmp">Image to detach from the view</param>
			virtual void vpDetachBmp(WriteableBitmap^ bmp)
			{
				if (m_callback)
					m_callback->cDetachBmp(bmp);
			}
			/// <summary>
			/// Attach data to an image
			/// </summary>
			/// <param name="bmp">Image to receive the data</param>
			/// <param name="arr">Data will be written to image</param>
			virtual void vpAttachBmp(WriteableBitmap^ bmp, const Array<byte>^ arr)
			{
				if (m_callback)
					m_callback->cAttachBmp(bmp, arr);
			}
			virtual CoreDispatcher^ vpGetDisp()
			{
				return Dispatcher;
			}
		protected:
			Windows::Foundation::Size MeasureOverride(Windows::Foundation::Size size) override
			{
				Windows::Foundation::Size ret = Canvas::MeasureOverride(size);
				if (m_layout)
				{
					double mul_val = 1.0 / m_zoom;
					ret.Width = m_layout->vGetLayW() * mul_val;
					ret.Height = m_layout->vGetLayH() * mul_val;
				}
				else
				{
					ret.Width = size.Width;
					ret.Height = 0;
				}
				return ret;
			}
			Windows::Foundation::Size ArrangeOverride(Windows::Foundation::Size finalSize) override
			{
				Windows::Foundation::Size ret = Canvas::ArrangeOverride(finalSize);
				if (m_layout)
				{
					double mul_val = 1.0 / m_zoom;
					ret.Width = m_layout->vGetLayW() * mul_val;
					ret.Height = m_layout->vGetLayH() * mul_val;
				}
				else
				{
					ret.Width = finalSize.Width;
					ret.Height = 0;
				}
				return ret;
			}
		private:
			Rectangle^ m_bound;
			PDFDoc^ m_doc;
			IPDFContentListener^ m_callback;
			double m_zoom;
			CRDVLayout* m_layout;
			int m_page_sel;
		};
		public interface class IPDFOPItem
		{
		public:
			/// <summary>
			/// Perform an undo to last operation
			/// </summary>
			/// <param name="doc">PDFDoc object to perform the undo operation</param>
			void undo(PDFDoc^ doc);
			/// <summary>
			/// Perform an redo to last operation
			/// </summary>
			/// <param name="doc">PDFDoc object to perform the redo operation</param>
			void redo(PDFDoc^ doc);
			/// <summary>
			/// Get current operation
			/// </summary>
			/// <returns></returns>
			int get_cur();
			/// <summary>
			/// Get 0 based page index of specified operation
			/// </summary>
			/// <param name="idx">Index of operation</param>
			/// <returns></returns>
			int get_pgno(int idx);
		};
		private ref class PDFOPItem : public IPDFOPItem
		{
		public:
			virtual void undo(PDFDoc^ doc) = 0;
			virtual void redo(PDFDoc^ doc) = 0;
			virtual int get_pgno(int idx) = 0;
			virtual int get_cur() { return m_pageno; }
		internal:
			PDFOPItem(int pageno, int idx)
			{
				m_pageno = pageno;
				m_idx = idx;
				m_prev = nullptr;
				m_next = nullptr;
			}

			int m_pageno;
			int m_idx;
			PDFOPItem^ m_prev;
			PDFOPItem^ m_next;
		};
		private ref class PDFOPAdd sealed: public PDFOPItem
		{
		public:
			PDFOPAdd(int pageno, PDFPage^ page, int idx)
				:PDFOPItem(pageno, idx)
			{
				m_hand = page->GetAnnot(idx)->Ref;
			}
			void undo(PDFDoc^ doc) override
			{
				PDFPage ^page = doc->GetPage(m_pageno);
				page->ObjsStart();
				PDFAnnot ^annot = page->GetAnnot(m_idx);
				annot->RemoveFromPage();
				page->Close();
			}
			void redo(PDFDoc^ doc) override
			{
				PDFPage ^page = doc->GetPage(m_pageno);
				page->ObjsStart();
				page->AddAnnot(m_hand, m_idx);
				page->Close();
			}
			int get_pgno(int idx) override
			{
				return m_pageno;
			}
		private:
			PDFRef m_hand;
		};
		private ref class PDFOPDel sealed: public PDFOPItem
		{
		public:
			PDFOPDel(int pageno, PDFPage^ page, int idx)
				:PDFOPItem(pageno, idx)
			{
				m_hand = page->GetAnnot(idx)->Ref;
			}
			void undo(PDFDoc^ doc) override
			{
				PDFPage^ page = doc->GetPage(m_pageno);
				page->ObjsStart();
				page->AddAnnot(m_hand, m_idx);
				page->Close();
			}
			void redo(PDFDoc^ doc) override
			{
				PDFPage^ page = doc->GetPage(m_pageno);
				page->ObjsStart();
				PDFAnnot^ annot = page->GetAnnot(m_idx);
				annot->RemoveFromPage();
				page->Close();
			}
			int get_pgno(int idx) override
			{
				return m_pageno;
			}
		private:
			PDFRef m_hand;
		};
		private ref class PDFOPMove sealed: public PDFOPItem
		{
		public:
			PDFOPMove(int src_pageno, PDFRect src_rect, int dst_pageno, int dst_idx, PDFRect dst_rect)
				:PDFOPItem(-1, dst_idx)
			{
				m_pageno0 = src_pageno;
				m_rect0 = src_rect;
				m_pageno1 = dst_pageno;
				m_rect1 = dst_rect;
			}
			void undo(PDFDoc^ doc) override
			{
				m_pageno = m_pageno0;
				if (m_pageno == m_pageno1)
				{
					PDFPage ^page = doc->GetPage(m_pageno);
					page->ObjsStart();
					PDFAnnot ^annot = page->GetAnnot(m_idx);
					annot->Rect = m_rect0;
					page->Close();
				}
				else
				{
					PDFPage ^page0 = doc->GetPage(m_pageno0);
					PDFPage ^page1 = doc->GetPage(m_pageno1);
					page1->ObjsStart();
					page0->ObjsStart();
					PDFAnnot ^annot = page1->GetAnnot(m_idx);
					annot->MoveToPage(page0, m_rect0);
					m_idx = page1->AnnotCount;
					page0->Close();
					page1->Close();
				}
			}
			void redo(PDFDoc^ doc) override
			{
				m_pageno = m_pageno1;
				if (m_pageno == m_pageno0)
				{
					PDFPage^ page = doc->GetPage(m_pageno);
					page->ObjsStart();
					PDFAnnot ^annot = page->GetAnnot(m_idx);
					annot->Rect = m_rect1;
					page->Close();
				}
				else
				{
					PDFPage^ page0 = doc->GetPage(m_pageno0);
					PDFPage^ page1 = doc->GetPage(m_pageno1);
					page1->ObjsStart();
					page0->ObjsStart();
					PDFAnnot ^annot = page0->GetAnnot(page0->AnnotCount - 1);
					annot->MoveToPage(page1, m_rect1);
					page0->Close();
					page1->Close();
				}
			}
			int get_pgno(int idx) override
			{
				if (idx == 0) return m_pageno0;
				else return m_pageno1;
			}
		private:
			int m_pageno0;
			int m_pageno1;
			PDFRect m_rect0;
			PDFRect m_rect1;
		};
		public ref class PDFOPStack sealed
		{
		public:
			PDFOPStack()
			{
				m_head = nullptr;
				m_tail = nullptr;
				m_cur = nullptr;
			}
			static IPDFOPItem^ new_add(int pageno, PDFPage^ page, int idx)
			{
				return ref new PDFOPAdd(pageno, page, idx);
			}
			static IPDFOPItem^ new_del(int pageno, PDFPage^ page, int idx)
			{
				return ref new PDFOPDel(pageno, page, idx);
			}
			static IPDFOPItem^ new_move(int src_pageno, PDFRect src_rect, int dst_pageno, int dst_idx, PDFRect dst_rect)
			{
				return ref new PDFOPMove(src_pageno, src_rect, dst_pageno, dst_idx, dst_rect);
			}
			void push(IPDFOPItem^ item)
			{
				PDFOPItem^ op = (PDFOPItem^)item;
				op->m_prev = m_cur;
				op->m_next = nullptr;
				if (m_cur)
					m_cur->m_next = op;
				else
					m_head = op;
				m_cur = op;
				m_tail = op;
			}
			IPDFOPItem ^undo()
			{
				if (m_cur == nullptr) return nullptr;
				IPDFOPItem^ cur = m_cur;
				m_cur = m_cur->m_prev;
				return cur;
			}
			IPDFOPItem ^redo()
			{
				if (m_cur == m_tail) return nullptr;
				if (m_cur) m_cur = m_cur->m_next;
				else m_cur = m_head;
				return m_cur;
			}
			bool has_undo()
			{
				return (m_cur != nullptr);
			}
			bool has_redo()
			{
				return (m_cur != m_tail);
			}
		internal:
			PDFOPItem ^m_cur;
			PDFOPItem ^m_head;
			PDFOPItem ^m_tail;
		private:
			~PDFOPStack()
			{
				m_head = nullptr;
				m_tail = nullptr;
				m_cur = nullptr;
			}
		};
		/*
		public ref class PDFViewer sealed : public IPDFContentListener
		{
		public:
			PDFViewer()
			{
				m_parent = nullptr;
				m_scroller = ref new ScrollViewer();
				m_content = ref new PDFVContent();
				m_canvas = ref new PDFVCanvas();
				m_scroller->Content = m_content;
				m_pageno = -1;
				m_scale = -1;

				m_doc = nullptr;
				m_oldZoom = -1;
				m_touched = false;
				m_modified = false;
				m_autofit = true;
				m_rects_cnt = 0;
				m_status = STA_NONE;
				m_sel = NULL;
				m_cur_mode = PDF_LAYOUT_MODE::layout_unknown;
				m_goto_pos.pageno = -1;
				m_goto_pos.x = 0;
				m_goto_pos.y = 0;
				m_keepauto = false;
			}
			property Boolean PDFModified
			{
				Boolean get() { return m_modified; }
				void set(Boolean val) { m_modified = val; }
			}
			property Boolean PDFAutoFit
			{
				Boolean get() { return m_autofit; }
				void set(Boolean val)
				{
					if ((m_autofit && val) || (!m_autofit && !val)) return;
					m_autofit = val;
					if (m_content)
					{
						m_keepauto = true;
						m_content->vSetAutoFit(val);
						m_content->vResize(m_scroller->ActualWidth, m_scroller->ActualHeight, m_scroller->ZoomFactor);
						double zoom = m_content->vGetScale();
						if (zoom > 0) m_scroller->ZoomToFactor(zoom);
					}
				}
			}
			static property float inkWidth
			{
				float get() { return sm_inkWidth; }
				void set(float val) { sm_inkWidth = val; }
			}
			static property unsigned int inkColor
			{
				unsigned int get() { return sm_inkColor; }
				void set(unsigned int val) { sm_inkColor = val; }
			}
			static property float rectWidth
			{
				float get() { return sm_rectWidth; }
				void set(float val) { sm_rectWidth = val; }
			}
			static property unsigned int rectColor
			{
				unsigned int get() { return sm_rectColor; }
				void set(unsigned int val) { sm_rectColor = val; }
			}
			static property float ovalWidth
			{
				float get() { return sm_ovalWidth; }
				void set(float val) { sm_ovalWidth = val; }
			}
			static property unsigned int ovalColor
			{
				unsigned int get() { return sm_ovalColor; }
				void set(unsigned int val) { sm_ovalColor = val; }
			}
			static property float lineWidth
			{
				float get() { return sm_lineWidth; }
				void set(float val) { sm_lineWidth = val; }
			}
			static property unsigned int lineColor
			{
				unsigned int get() { return sm_lineColor; }
				void set(unsigned int val) { sm_lineColor = val; }
			}
			static property unsigned int textColor
			{
				unsigned int get() { return sm_textColor; }
				void set(unsigned int val) { sm_textColor = val; }
			}
			static property PDF_LAYOUT_MODE viewMode
			{
				PDF_LAYOUT_MODE get() { return sm_viewMode; }
				void set(PDF_LAYOUT_MODE vmode) { sm_viewMode = vmode; }
			}
			bool PDFOpen(RelativePanel^ parent, PDFDoc^ doc, PDF_LAYOUT_MODE lmode, IPDFViewListener ^listener)
			{
				if (!parent || !doc) return false;
				m_parent = parent;
				m_doc = doc;
				m_listener = listener;
				m_pageno = -1;
				m_scale = -1;

				Windows::UI::Color clr;
				clr.A = 255;
				clr.R = 224;
				clr.G = 224;
				clr.B = 224;
				m_parent->Background = ref new SolidColorBrush(clr);
				
				//clr.R = 255;
				//m_content->Background = ref new SolidColorBrush(clr);
				m_parent->Children->Append(m_scroller);
				m_parent->Children->Append(m_canvas);
				m_scroller->SetValue(RelativePanel::AlignLeftWithPanelProperty, true);
				m_scroller->SetValue(RelativePanel::AlignTopWithPanelProperty, true);
				m_scroller->SetValue(RelativePanel::AlignRightWithPanelProperty, true);
				m_scroller->SetValue(RelativePanel::AlignBottomWithPanelProperty, true);
				m_canvas->SetValue(RelativePanel::AlignLeftWithPanelProperty, true);
				m_canvas->SetValue(RelativePanel::AlignTopWithPanelProperty, true);
				m_canvas->SetValue(RelativePanel::AlignRightWithPanelProperty, true);
				m_canvas->SetValue(RelativePanel::AlignBottomWithPanelProperty, true);

				m_cur_mode = lmode;
				m_content->vOpen(m_doc, lmode, this);
				//all coordinate events shall from parent.
				cookie_pointPress = m_parent->PointerPressed += ref new PointerEventHandler(this, &PDFViewer::vOnTouchDown);
				cookie_pointMove = m_parent->PointerMoved += ref new PointerEventHandler(this, &PDFViewer::vOnTouchMove);;
				cookie_pointUp = m_parent->PointerReleased += ref new PointerEventHandler(this, &PDFViewer::vOnTouchUp);;
				cookie_pointCancel = m_parent->PointerCanceled += ref new PointerEventHandler(this, &PDFViewer::vOnTouchUp);;
				cookie_pointerExited = m_parent->PointerExited += ref new PointerEventHandler(this, &PDFViewer::vOnTouchUp);;
				cookie_tapped = m_parent->Tapped += ref new TappedEventHandler(this, &PDFViewer::vOnTapped);
				cookie_doubleTapped = m_parent->DoubleTapped += ref new DoubleTappedEventHandler(this, &PDFViewer::vOnDoubleTapped);

				cookie_sizeChanged = m_scroller->SizeChanged += ref new SizeChangedEventHandler(this, &PDFViewer::vOnSizeChanged);
				cookie_viewChanged = m_scroller->ViewChanged += ref new EventHandler<ScrollViewerViewChangedEventArgs^>(this, &PDFViewer::vOnViewChanged);
				//m_view->LayoutUpdated += OnLayout;
				m_scroller->ZoomMode = ZoomMode::Enabled;
				m_scroller->IsZoomChainingEnabled = false;
				m_scroller->MinZoomFactor = 0.3;
				m_scroller->MaxZoomFactor = 10;
				m_scroller->HorizontalScrollBarVisibility = ScrollBarVisibility::Visible;
				m_scroller->VerticalScrollBarVisibility = ScrollBarVisibility::Visible;
				m_scroller->IsHoldingEnabled = true;
				m_scroller->IsScrollInertiaEnabled = true;
				m_scroller->IsHitTestVisible = true;
				if (m_scroller->ActualWidth > 0 && m_scroller->ActualHeight > 0)
				{
					bool val = m_autofit;
					m_autofit = !val;
					PDFAutoFit = val;
					vDraw();
				}
				else m_content->vSetAutoFit(m_autofit);
				return true;
			}
			void PDFSetView(PDF_LAYOUT_MODE lmode)
			{
				if (m_cur_mode == lmode) return;
				m_cur_mode = lmode;
				double vx = m_canvas->ActualWidth * 0.5;
				double vy = m_canvas->ActualHeight * 0.5;
				PDFPos pos = m_content->vGetPos(m_canvas->ActualWidth * 0.5, m_canvas->ActualHeight * 0.5);
				m_scroller->ZoomToFactor(1);
				m_content->vSetView(lmode);
				if (m_scroller->ActualWidth > 0 && m_scroller->ActualHeight > 0)
					m_content->vResize(m_scroller->ActualWidth, m_scroller->ActualHeight, m_scroller->ZoomFactor);
				m_content->vSetPos(vx, vy, pos);//will update scroll view.
				vDraw();
			}
			void PDFGotoPage(int pageno)
			{
				if (m_content && m_content->vCanSetPos())
				{
					PDFPos pos;
					pos.pageno = pageno;
					pos.x = 2;
					pos.y = m_doc->GetPageHeight(pageno) + 2;
					m_content->vSetPos(0, 0, pos);//will update scroll view.
				}
				else
				{
					m_goto_pos.pageno = pageno;
					m_goto_pos.x = 2;
					m_goto_pos.y = m_doc->GetPageHeight(pageno) + 2;
				}
			}
			int PDFGetCurPageNo()
			{
				return m_pageno;
			}
			void PDFClose()
			{
				m_scroller->SizeChanged -= cookie_sizeChanged;
				m_scroller->ViewChanged -= cookie_viewChanged;

				m_listener = nullptr;
				m_scroller->Content = nullptr;
				m_content->vClose();
				m_canvas->clear();
				m_doc = nullptr;
				m_touched = false;
				m_modified = false;
				m_rects_cnt = 0;
				if (m_sel) delete m_sel;
				m_cur_mode = PDF_LAYOUT_MODE::layout_unknown;
				if (m_parent)
				{
					m_parent->PointerPressed -= cookie_pointPress;
					m_parent->PointerMoved -= cookie_pointMove;
					m_parent->PointerReleased -= cookie_pointUp;
					m_parent->PointerCanceled -= cookie_pointCancel;
					m_parent->PointerExited -= cookie_pointerExited;
					m_parent->Tapped -= cookie_tapped;
					m_parent->DoubleTapped -= cookie_doubleTapped;
					m_parent->Children->Clear();
					m_parent = nullptr;
				}
			}
			void PDFRenderPage(int pageno)
			{
				m_content->vRenderPage(m_content->vGetPage(pageno));
			}
			void PDFSelStart()
			{
				if (m_status == STA_NONE)
				{
					m_status = STA_SELECT;
					m_scroller->IsEnabled = false;
				}
			}
			void PDFSelEnd()
			{
				if (m_status == STA_SELECT)
				{
					PDFSelCancel();
					m_scroller->IsEnabled = true;
					m_status = STA_NONE;
				}
			}
			void PDFSelCancel()
			{
				if (m_status == STA_SELECT)
				{
					delete m_sel;
					m_sel = NULL;
					m_status = STA_NONE;
					vDraw();
				}
			}
			String ^PDFSelGetText()
			{
				if (m_status != STA_SELECT) return nullptr;
				if (m_sel)
					return m_sel->GetSelString();
				return nullptr;
			}
			bool PDFSelSetMarkup(unsigned int color, int type)
			{
				if (m_status != STA_SELECT) return false;
				int pageno = m_sel->GetPageNo();
				if (pageno >= 0)
				{
					m_sel->SetSelMarkup(color, type);
					m_content->vRenderPage(m_content->vGetPage(pageno));
					vDraw();
					m_modified = true;
					if (m_listener) m_listener->OnPDFPageUpdated(m_sel->GetPageNo());
					return true;
				}
				return false;
			}
			void PDFAnnotPerform()
			{
				if (m_status != STA_ANNOT) return;
				int pageno = m_annot->Dest;
				if (pageno >= 0)//goto page
				{
					if (m_listener)
						m_listener->OnPDFAnnotGoto(pageno);
					PDFAnnotEnd();
					return;
				}
				if (m_annot->IsRemoteDest)
				{
					if (m_listener)
						m_listener->OnPDFAnnotRemoteDest(m_annot->RemoteDest);
					PDFAnnotEnd();
					return;
				}
				if (m_annot->IsFileLink)
				{
					if (m_listener)
						m_listener->OnPDFAnnotFileLink(m_annot->FileLink);
					PDFAnnotEnd();
					return;
				}
				if (m_annot->IsURI)//open url
				{
					if (m_listener)
						m_listener->OnPDFAnnotURI(m_annot->URI);
					PDFAnnotEnd();
					return;
				}
				if (m_annot->RichMediaItemCount > 0)     // Check if the annotation is a rich media container
				{
					if (m_listener)
						m_listener->OnPDFAnnotRichMedia(m_annot);
					PDFAnnotEnd();
					return;
				}
				if (m_annot->IsPopup && m_annot->Type == 1) //Nermeen add type check as it enters also for highlight annot
				{
					//popup dialog to show text and subject.
					//nuri is text content.
					//subj is subject string.

					if (m_listener)
						m_listener->OnPDFAnnotPopup(m_annot, m_annot->PopupSubject, m_annot->PopupText);
					PDFAnnotEnd();
					return;
				}
				PDFAnnotEnd();
				return;
			}
			void PDFAnnotRemove()
			{
				if (m_status != STA_ANNOT) return;
				if (m_annot->RemoveFromPage())
				{
					m_content->vRenderPage(m_content->vGetPage(m_annot_pos.pageno));
					vDraw();
				}
				PDFAnnotEnd();
			}
			void PDFAnnotEnd()
			{
				if (m_status != STA_ANNOT) return;
				m_status = STA_NONE;
				m_scroller->IsEnabled = true;
				m_annot = nullptr;
				vDraw();
				if (m_listener)
					m_listener->OnPDFAnnotEnd();
			}
			bool PDFEllipseStart()
			{
				if (!m_doc || !m_doc->CanSave) return false;
				if (m_status == STA_NONE)
				{
					m_status = STA_ELLIPSE;
					m_rects_cnt = 0;
					m_scroller->IsEnabled = false;
					return true;
				}
				return false;
			}
			void PDFEllipseCancel()
			{
				if (m_status == STA_ELLIPSE)
				{
					m_scroller->IsEnabled = true;
					m_rects_cnt = 0;
					m_status = STA_NONE;
					vDraw();
				}
			}
			void PDFEllipseEnd()
			{
				if (m_status == STA_ELLIPSE)
				{
					CRDVPage *pages[128];
					int cur;
					int end;
					int pages_cnt = 0;
					int pt_cur = 0;
					int pt_end = m_rects_cnt * 2;
					while (pt_cur < pt_end)
					{
						PDFRect rect;
						PDFPoint pt0 = m_rects[pt_cur];
						PDFPoint pt1 = m_rects[pt_cur + 1];
						int pageno = m_content->vGetPage(pt0.x, pt0.y);
						if (pageno >= 0)
						{
							CRDVPage *vpage = (CRDVPage*)m_content->vGetPage(pageno);
							cur = 0;
							end = pages_cnt;
							while (cur < end)
							{
								if (pages[cur] == vpage) break;
								cur++;
							}
							if (cur >= end)
							{
								pages[cur] = vpage;
								pages_cnt++;
							}
							if (pt0.x > pt1.x)
							{
								rect.right = pt0.x;
								rect.left = pt1.x;
							}
							else
							{
								rect.left = pt0.x;
								rect.right = pt1.x;
							}
							if (pt0.y > pt1.y)
							{
								rect.bottom = pt0.y;
								rect.top = pt1.y;
							}
							else
							{
								rect.top = pt0.y;
								rect.bottom = pt1.y;
							}
							PDFPage ^page = m_doc->GetPage(pageno);
							page->ObjsStart();
							PDFMatrix ^mat = vpage->CreateInvertMatrix(m_scroller->HorizontalOffset, m_scroller->VerticalOffset);
							rect = mat->TransformRect(rect);
							page->AddAnnotEllipse(rect, sm_ovalWidth / vpage->GetScale(), sm_ovalColor, 0);
						}
						pt_cur += 2;
					}
					if (m_rects_cnt != 0)
						m_modified = true;
					m_rects_cnt = 0;
					m_status = STA_NONE;

					cur = 0;
					end = pages_cnt;
					while (cur < end)
					{
						m_content->vRenderPage((long long)pages[cur]);
						if (m_listener) m_listener->OnPDFPageUpdated(pages[cur]->GetPageNo());
						cur++;
					}
					vDraw();
					m_scroller->IsEnabled = true;
				}
			}
			bool PDFRectStart()
			{
				if (!m_doc || !m_doc->CanSave) return false;
				if (m_status == STA_NONE)
				{
					m_status = STA_RECT;
					m_rects_cnt = 0;
					m_scroller->IsEnabled = false;
					return true;
				}
				return false;
			}
			void PDFRectCancel()
			{
				if (m_status == STA_RECT)
				{
					m_scroller->IsEnabled = true;
					m_rects_cnt = 0;
					m_status = STA_NONE;
					vDraw();
				}
			}
			void PDFRectEnd()
			{
				if (m_status == STA_RECT)
				{
					CRDVPage* pages[128];
					int cur;
					int end;
					int pages_cnt = 0;
					int pt_cur = 0;
					int pt_end = m_rects_cnt * 2;
					while (pt_cur < pt_end)
					{
						PDFRect rect;
						PDFPoint pt0 = m_rects[pt_cur];
						PDFPoint pt1 = m_rects[pt_cur + 1];
						int pageno = m_content->vGetPage(pt0.x, pt0.y);
						if (pageno >= 0)
						{
							CRDVPage* vpage = (CRDVPage*)m_content->vGetPage(pageno);
							cur = 0;
							end = pages_cnt;
							while (cur < end)
							{
								if (pages[cur] == vpage) break;
								cur++;
							}
							if (cur >= end)
							{
								pages[cur] = vpage;
								pages_cnt++;
							}
							if (pt0.x > pt1.x)
							{
								rect.right = pt0.x;
								rect.left = pt1.x;
							}
							else
							{
								rect.left = pt0.x;
								rect.right = pt1.x;
							}
							if (pt0.y > pt1.y)
							{
								rect.bottom = pt0.y;
								rect.top = pt1.y;
							}
							else
							{
								rect.top = pt0.y;
								rect.bottom = pt1.y;
							}
							PDFPage^ page = m_doc->GetPage(pageno);
							page->ObjsStart();
							PDFMatrix^ mat = vpage->CreateInvertMatrix(m_scroller->HorizontalOffset, m_scroller->VerticalOffset);
							rect = mat->TransformRect(rect);
							page->AddAnnotRect(rect, sm_rectWidth / vpage->GetScale(), sm_rectColor, 0);
						}
						pt_cur += 2;
					}
					if (m_rects_cnt != 0)
						m_modified = true;
					m_rects_cnt = 0;
					m_status = STA_NONE;

					cur = 0;
					end = pages_cnt;
					while (cur < end)
					{
						m_content->vRenderPage((long long)pages[cur]);
						if (m_listener) m_listener->OnPDFPageUpdated(pages[cur]->GetPageNo());
						cur++;
					}
					vDraw();
					m_scroller->IsEnabled = true;
				}
			}
			bool PDFInkStart()
			{
				if (!m_doc || !m_doc->CanSave) return false;
				if (m_status == STA_NONE)
				{
					m_scroller->IsEnabled = false;
					m_ink = nullptr;
					m_ink_path = nullptr;
					m_ink_pos = 0;
					m_status = STA_INK;
					return true;
				}
				return false;
			}
			void PDFInkCancel()
			{
				if (m_status == STA_INK)
				{
					m_scroller->IsEnabled = true;
					m_status = STA_NONE;
					m_ink = nullptr;
					m_ink_path = nullptr;
					m_ink_pos = 0;
					vDraw();
				}
			}
			void PDFInkEnd()
			{
				if (m_status == STA_INK)
				{
					if (m_ink)
					{
						int pageno = m_content->vGetPage(m_hold_x, m_hold_y);
						if (pageno >= 0)
						{
							CRDVPage *vpage = (CRDVPage*)m_content->vGetPage(pageno);
							PDFMatrix^ mat = vpage->CreateInvertMatrix(m_scroller->HorizontalOffset, m_scroller->VerticalOffset);
							PDFPage ^page = m_doc->GetPage(pageno);
							page->ObjsStart();
							mat->TransformInk(m_ink);
							page->AddAnnotInk(m_ink);
							m_content->vRenderPage((long long)vpage);
							if (m_listener) m_listener->OnPDFPageUpdated(pageno);
							m_modified = true;
						}
					}
					m_scroller->IsEnabled = true;
					m_status = STA_NONE;
					m_ink = nullptr;
					m_ink_path = nullptr;
					m_ink_pos = 0;
					vDraw();
				}
			}
			bool PDFLineStart()
			{
				if (!m_doc || !m_doc->CanSave) return false;
				if (m_status == STA_NONE)
				{
					m_status = STA_LINE;
					m_rects_cnt = 0;
					m_scroller->IsEnabled = false;
					return true;
				}
				return false;
			}
			void PDFLineCancel()
			{
				if (m_status == STA_RECT)
				{
					m_scroller->IsEnabled = true;
					m_rects_cnt = 0;
					m_status = STA_LINE;
					vDraw();
				}
			}
			void PDFLineEnd()
			{
				if (m_status == STA_LINE)
				{
					CRDVPage* pages[128];
					int cur;
					int end;
					int pages_cnt = 0;
					int pt_cur = 0;
					int pt_end = (m_rects_cnt << 1);
					while (pt_cur < pt_end)
					{
						PDFPoint pt0 = m_rects[pt_cur];
						PDFPoint pt1 = m_rects[pt_cur + 1];
						int pageno = m_content->vGetPage(pt0.x, pt0.y);
						if (pageno >= 0)
						{
							CRDVPage* vpage = (CRDVPage*)m_content->vGetPage(pageno);
							cur = 0;
							end = pages_cnt;
							while (cur < end)
							{
								if (pages[cur] == vpage) break;
								cur++;
							}
							if (cur >= end)
							{
								pages[cur] = vpage;
								pages_cnt++;
							}
							PDFPage ^page = m_doc->GetPage(pageno);
							page->ObjsStart();
							PDFMatrix ^mat = vpage->CreateInvertMatrix(m_scroller->HorizontalOffset, m_scroller->VerticalOffset);
							pt0 = mat->TransformPoint(pt0);
							pt1 = mat->TransformPoint(pt1);
							page->AddAnnotLine(pt0.x, pt0.y, pt1.x, pt1.y, 1, 0, sm_lineWidth / vpage->GetScale(), sm_lineColor, 0);
						}
						pt_cur += 2;
					}
					if (m_rects_cnt != 0)
						m_modified = true;
					m_rects_cnt = 0;
					m_status = STA_NONE;

					cur = 0;
					end = pages_cnt;
					while (cur < end)
					{
						m_content->vRenderPage((long long)pages[cur]);
						if (m_listener) m_listener->OnPDFPageUpdated(pages[cur]->GetPageNo());
						cur++;
					}
					vDraw();
					m_scroller->IsEnabled = true;
				}
			}
			bool PDFNoteStart()
			{
				if (!m_doc || !m_doc->CanSave) return false;
				if (m_status == STA_NONE)
				{
					m_scroller->IsEnabled = false;
					m_status = STA_NOTE;
				}
				return true;
			}

			void PDFNoteRemoveLast()
			{
				if (m_notes_cnt <= 0) return;
				CRDVPage *vpage = m_notes[m_notes_cnt - 1].vpage;
				int index = m_notes[m_notes_cnt - 1].index;
				PDFPage ^page = m_doc->GetPage(vpage->GetPageNo());
				if (page)
				{
					page->ObjsStart();
					PDFAnnot ^annot = page->GetAnnot(index);
					annot->RemoveFromPage();
					m_notes[m_notes_cnt - 1].vpage = NULL;
					m_notes_cnt--;
					m_content->vRenderPage((long long)vpage);
					vDraw();
				}
			}
			void PDFNoteCancel()
			{
				if (m_status == STA_NOTE)
				{
					m_scroller->IsEnabled = true;
					m_status = STA_NONE;
					CRDVPage *vpages[256];
					int vpages_cnt = 0;
					int index;
					for (int cur = m_notes_cnt - 1; cur >= 0; cur--)
					{
						CRDVPage *vpage = m_notes[cur].vpage;
						for (index = 0; index < vpages_cnt; index++)
						{
							if (vpages[index] == vpage) break;
						}
						if (index >= vpages_cnt)
						{
							vpages[vpages_cnt] = vpage;
							vpages_cnt++;
						}
						PDFPage ^page = m_doc->GetPage(vpage->GetPageNo());
						if (page)
						{
							page->ObjsStart();
							PDFAnnot ^annot = page->GetAnnot(m_notes[cur].index);
							annot->RemoveFromPage();
						}
						m_notes[cur].vpage = NULL;
					}
					m_notes_cnt = 0;
					for (index = 0; index < vpages_cnt; index++)
					{
						m_content->vRenderPage((long long)vpages[index]);
					}
					vDraw();
				}
			}
			void PDFNoteEnd()
			{
				if (m_status == STA_NOTE)
				{
					m_scroller->IsEnabled = true;
					m_status = STA_NONE;
					if (m_notes_cnt > 0)
						m_modified = true;
					for (int cur = m_notes_cnt - 1; cur >= 0; cur--)
						m_notes[cur].vpage = NULL;
					m_notes_cnt = 0;
					vDraw();
				}
			}
			bool PDFTextEditStart()
			{
				if (!m_doc || !m_doc->CanSave || !m_content) return false;
				if (m_status == STA_NONE)
				{
					m_status = STA_TEXT_EDIT;
					m_rects_cnt = 0;
					m_scroller->IsEnabled = false;
					return true;
				}
				return false;
			}

			void PDFTextEditCancel()
			{
				if (m_status == STA_TEXT_EDIT)
				{
					m_scroller->IsEnabled = true;
					m_rects_cnt = 0;
					m_status = STA_NONE;
					vDraw();
				}
			}

			void PDFTextEditEnd()
			{
				if (m_status != STA_TEXT_EDIT) return;
				CRDVPage *pages[128];
				int cur;
				int end;
				int pages_cnt = 0;
				int pt_cur = 0;
				int pt_end = m_rects_cnt * 2;
				double tmp = 1 / m_scroller->ZoomFactor;
				while (pt_cur < pt_end)
				{
					PDFRect rect;
					PDFPoint pt0 = m_rects[pt_cur];
					PDFPoint pt1 = m_rects[pt_cur + 1];
					int pageno = m_content->vGetPage(pt0.x, pt0.y);
					if (pageno >= 0)
					{
						CRDVPage *vpage = (CRDVPage*)m_content->vGetPage(pageno);
						cur = 0;
						end = pages_cnt;
						while (cur < end)
						{
							if (pages[cur] == vpage) break;
							cur++;
						}
						if (cur >= end)
						{
							pages[cur] = vpage;
							pages_cnt++;
						}
						if (pt0.x > pt1.x)
						{
							rect.right = pt0.x * tmp;
							rect.left = pt1.x * tmp;
						}
						else
						{
							rect.left = pt0.x * tmp;
							rect.right = pt1.x * tmp;
						}
						if (pt0.y > pt1.y)
						{
							rect.bottom = pt0.y * tmp;
							rect.top = pt1.y * tmp;
						}
						else
						{
							rect.top = pt0.y * tmp;
							rect.bottom = pt1.y * tmp;
						}
						PDFPage ^page = m_doc->GetPage(pageno);
						page->ObjsStart();
						PDFMatrix^ mat = vpage->CreateInvertMatrix(m_scroller->HorizontalOffset, m_scroller->VerticalOffset);
						rect = mat->TransformRect(rect);
						page->AddAnnotEditbox(rect, 0xFF0000FF, 2, 0x00FFFFFF, 12.0f, sm_textColor);
						//page.AddAnnotRect(rect, (rectWidth * tmp) / vpage.GetScale(), rectColor, 0);
					}
					pt_cur += 2;
				}
				if (m_rects_cnt != 0)
					m_modified = true;
				m_rects_cnt = 0;
				m_status = STA_NONE;

				cur = 0;
				end = pages_cnt;
				while (cur < end)
				{
					m_content->vRenderPage((long long)pages[cur]);
					cur++;
				}
				vDraw();
				m_scroller->IsEnabled = true;
			}

			void PDFFindStart(String^ pat, bool match_case, bool whole_word)
			{
				m_content->vFindStart(pat, match_case, whole_word);
			}
			void PDFFind(int dir)
			{
				m_content->vFind(dir);
			}
			void PDFFindEnd()
			{
				m_content->vFindEnd();
			}
			void cFillRect(const PDFRect &rect, Windows::UI::Color clr) override//to fill rectangles
			{
				m_canvas->fill_rect(rect, clr);
			}
			void cSetPos(double vx, double vy) override//to set scroll position.
			{
				m_scroller->ChangeView(vx, vy, (float)m_content->vGetScale(), true);
			}
			void cFound(bool found) override
			{
				if (found) vDraw();
				if (m_listener) m_listener->OnPDFFound(found);
			}
		private:
			~PDFViewer()
			{
				PDFClose();
			}
			//Windows::UI::Xaml::SizeChangedEventHandler
			void vOnSizeChanged(Object ^sender, SizeChangedEventArgs ^e)
			{
				if (!m_content) return;
				if (m_autofit)
				{
					m_autofit = false;
					PDFAutoFit = true;
				}
				else
				{
					m_content->vResize(e->NewSize.Width, e->NewSize.Height, m_scroller->ZoomFactor);
					if (m_goto_pos.pageno >= 0)
					{
						int pageno = m_goto_pos.pageno;
						PDFViewer^ viewer = this;
						m_scroller->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([viewer, pageno]() { viewer->PDFGotoPage(pageno); }));
						m_goto_pos.pageno = -1;
						m_goto_pos.x = 0;
						m_goto_pos.y = 0;
					}
					vDraw();
				}
			}
			void vOnTouchDown(Object ^sender, PointerRoutedEventArgs ^e)
			{
				PointerPoint ^ppt = e->GetCurrentPoint(m_canvas);
				//m_parent->CapturePointer(e->Pointer);
				Point pt = ppt->Position;
				pt.X = to_contx(pt.X);
				pt.Y = to_conty(pt.Y);
				m_touched = true;
				if (OnSelTouchBegin(pt)) return;
				if (OnAnnotTouchBegin(pt)) return;
				if (OnNoteTouchBegin(pt)) return;
				if (OnInkTouchBegin(pt)) return;
				if (OnRectTouchBegin(pt)) return;
				if (OnEllipseTouchBegin(pt)) return;
				if (OnLineTouchBegin(pt)) return;
				if (OnEditTextBoxTouchBegin(pt)) return;
				if (e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
				{
					if (ppt->Properties->IsLeftButtonPressed)
					{
						OnNoneTouchBegin(pt, ppt->Timestamp);
					}
				}
			}
			void vOnTouchMove(Object^ sender, PointerRoutedEventArgs^ e)
			{
				if (m_touched)
				{
					PointerPoint^ ppt = e->GetCurrentPoint(m_canvas);
					Point pt = ppt->Position;
					pt.X = to_contx(pt.X);
					pt.Y = to_conty(pt.Y);
					if (OnSelTouchMove(pt)) return;
					if (OnAnnotTouchMove(pt)) return;
					if (OnNoteTouchMove(pt)) return;
					if (OnInkTouchMove(pt)) return;
					if (OnRectTouchMove(pt)) return;
					if (OnEllipseTouchMove(pt)) return;
					if (OnLineTouchMove(pt)) return;
					if (OnEditTextBoxTouchMove(pt)) return;
					OnNoneTouchMove(pt, ppt->Timestamp);
				}
			}
			void vOnTouchUp(Object^ sender, PointerRoutedEventArgs^ e)
			{
				if (m_touched)
				{
					//m_parent->ReleasePointerCapture(e->Pointer);
					PointerPoint^ ppt = e->GetCurrentPoint(m_canvas);
					Point pt = ppt->Position;
					pt.X = to_contx(pt.X);
					pt.Y = to_conty(pt.Y);
					m_touched = false;
					if (OnSelTouchEnd(pt)) return;
					if (OnAnnotTouchEnd(pt)) return;
					if (OnNoteTouchEnd(pt)) return;
					if (OnInkTouchEnd(pt)) return;
					if (OnRectTouchEnd(pt)) return;
					if (OnEllipseTouchEnd(pt)) return;
					if (OnLineTouchEnd(pt)) return;
					if (OnEditTextBoxTouchEnd(pt)) return;
				}
			}
			void vOnTapped(Object^ sender, TappedRoutedEventArgs ^e)
			{
				if (m_status == STA_NONE)
				{
					Point point = e->GetPosition(m_canvas);
					point.X = to_contx(point.X);
					point.Y = to_conty(point.Y);
					m_annot_pos = m_content->vGetPos(point.X, point.Y);
					if (m_annot_pos.pageno >= 0)
					{
						CRDVPage *vpage = (CRDVPage*)m_content->vGetPage(m_annot_pos.pageno);
						if (!vpage)//shall not happen
						{
							if (m_listener) m_listener->OnPDFSingleTapped(point.X, point.Y);
							return;
						}
						PDFPage ^page = m_doc->GetPage(vpage->GetPageNo());
						if (!page)
						{
							if (m_listener)
							{
								m_listener->OnPDFPageTapped(m_annot_pos.pageno);
								m_listener->OnPDFSingleTapped(point.X, point.Y);
							}
							return;
						}
						page->ObjsStart();
						m_annot = page->GetAnnot(m_annot_pos.x, m_annot_pos.y);
						if (m_annot)//enter annotation status.
						{
							int itmp = m_annot->GetCheckStatus();
							if (m_doc->CanSave && itmp >= 0)
							{
								switch (itmp)
								{
								case 0:
									m_annot->SetCheckValue(true);
									break;
								case 1:
									m_annot->SetCheckValue(false);
									break;
								case 2:
								case 3:
									m_annot->SetRadio();
									break;
								}
								m_content->vRenderPage((long long)vpage);
								//m_status still is STA_NONE, PDFAnnotEnd() does nothing.
								//PDFAnnotEnd();
								vDraw();
								m_modified = true;
								if (m_listener) m_listener->OnPDFPageUpdated(m_annot_pos.pageno);
								return;
							}
							m_scroller->IsEnabled = false;
							m_status = STA_ANNOT;
							m_annot_rect = m_annot->Rect;
							m_annot_rect.left = vpage->GetLeft() + vpage->ToDIBX(m_annot_rect.left);
							m_annot_rect.right = vpage->GetLeft() + vpage->ToDIBX(m_annot_rect.right);
							float tmp = m_annot_rect.top;
							m_annot_rect.top = vpage->GetTop() + vpage->ToDIBY(m_annot_rect.bottom);
							m_annot_rect.bottom = vpage->GetTop() + vpage->ToDIBY(tmp);

							tmp = m_scroller->HorizontalOffset;
							m_annot_rect.left -= tmp;
							m_annot_rect.right -= tmp;
							tmp = m_scroller->VerticalOffset;
							m_annot_rect.top -= tmp;
							m_annot_rect.bottom -= tmp;

							m_shold_x = m_hold_x;
							m_shold_y = m_hold_y;
							vDraw();
							if (m_listener)
							{
								m_listener->OnPDFPageTapped(vpage->GetPageNo());
								m_listener->OnPDFAnnotClicked(page, m_annot_pos.pageno, m_annot, m_annot_rect);
							}
						}
						else
						{
							if (m_listener)
							{
								m_listener->OnPDFPageTapped(vpage->GetPageNo());
								m_listener->OnPDFSingleTapped(point.X, point.Y);
							}
						}
					}
				}
			}
			void vOnDoubleTapped(Object ^sender, DoubleTappedRoutedEventArgs ^e)
			{
				if (m_status == STA_NONE)
				{
					if (m_scroller->ZoomFactor * 1.2 < m_scroller->MaxZoomFactor)
					{
						Point point = e->GetPosition(m_canvas);
						point.X = to_contx(point.X);
						point.Y = to_conty(point.Y);
						double offsetx = point.X * 0.2;
						double offsety = point.Y * 0.2;
						m_scroller->ChangeView(m_scroller->HorizontalOffset * 1.2 + offsetx, m_scroller->VerticalOffset * 1.2 + offsety, m_scroller->ZoomFactor * 1.2f, false);
						//vSetPos(pos, (float)point.X, (float)point.Y);
					}
					else
					{
						double tmp = 1 / m_scroller->ZoomFactor;
						m_scroller->ChangeView(m_scroller->HorizontalOffset * tmp, m_scroller->VerticalOffset * tmp, 1.0f, false);
					}
					m_scroller->UpdateLayout();
				}
			}
			void vOnViewChanged(Object^ sender, ScrollViewerViewChangedEventArgs^ e)
			{
				if (!m_content) return;
				if (m_oldZoom < 0)
				{
					m_oldZoom = m_scroller->ZoomFactor;
				}
				else if (m_oldZoom != m_scroller->ZoomFactor)
				{
					m_touched = false;
					if (m_status == STA_NONE)
					{
						m_content->vZoomStart();
						m_status = STA_ZOOM;
					}
					if (m_status == STA_ZOOM)
					{
						m_content->vZoomSet(m_scroller->ZoomFactor);
						m_content->vResize(m_scroller->ActualWidth, m_scroller->ActualHeight, m_scroller->ZoomFactor);
						if (m_keepauto)
						{
							m_content->vSetAutoFit(true);
							m_keepauto = false;
							m_content->vZoomConfirm();
							m_status = STA_NONE;
						}
						else
							m_autofit = false;
					}
					m_oldZoom = m_scroller->ZoomFactor;
				}
				else
				{
					if (m_status == STA_ZOOM)
					{
						m_content->vZoomConfirm();
						m_status = STA_NONE;
					}
				}
				vDraw();
			}
			enum PDFV_STATUS
			{
				STA_NONE = 0,
				STA_ZOOM = 1,
				STA_SELECT = 2,
				STA_ANNOT = 3,
				STA_NOTE = 4,
				STA_INK = 5,
				STA_RECT = 6,
				STA_ELLIPSE = 7,
				STA_LINE = 8,
				STA_TEXT_EDIT = 9
			}m_status;
			CRDVSel *m_sel;
			PDFPos m_goto_pos;

			//map coordinate from ScrollViewer to content View
			inline double to_contx(double x)
			{
				double tdx = m_content->ActualWidth * m_scroller->ZoomFactor;
				if (tdx < m_canvas->ActualWidth)
					return x - (m_canvas->ActualWidth - tdx) * 0.5;
				else
					return x;
			}
			inline double to_conty(double y)
			{
				double tdy = m_content->ActualHeight * m_scroller->ZoomFactor;
				double tdx = m_content->ActualWidth * m_scroller->ZoomFactor;
				if (tdx >= m_parent->ActualWidth)
					return y;
				else if (tdy < m_canvas->ActualHeight)
					return y - (m_canvas->ActualHeight - tdy) * 0.5;
				else
					return y;
			}
			inline double to_canvasx(double x)
			{
				double tdx = m_content->ActualWidth * m_scroller->ZoomFactor;
				if (tdx < m_canvas->ActualWidth)
					return x + (m_canvas->ActualWidth - tdx) * 0.5;
				else
					return x;
			}
			inline double to_canvasy(double y)
			{
				double tdy = m_content->ActualHeight * m_scroller->ZoomFactor;
				double tdx = m_content->ActualWidth * m_scroller->ZoomFactor;
				if (tdx >= m_parent->ActualWidth)
					return y;
				else if (tdy < m_canvas->ActualHeight)
					return y + (m_canvas->ActualHeight - tdy) * 0.5;
				else
					return y;
			}
			bool OnSelTouchBegin(Point point)
			{
				if (m_status != STA_SELECT) return false;
				m_hold_x = point.X;
				m_hold_y = point.Y;
				m_annot_pos = m_content->vGetPos(m_hold_x, m_hold_y);
				delete m_sel;
				m_sel = new CRDVSel(m_doc, m_annot_pos.pageno, to_contx(m_scroller->HorizontalOffset), to_conty(m_scroller->VerticalOffset));
				return true;
			}
			bool OnSelTouchMove(Point point)
			{
				if (m_status != STA_SELECT) return false;
				CRDVPage* vpage = (CRDVPage*)m_content->vGetPage(m_annot_pos.pageno);
				double pdfx = vpage->GetPDFX(point.X + m_scroller->HorizontalOffset);
				double pdfy = vpage->GetPDFY(point.Y + m_scroller->VerticalOffset);
				m_sel->SetSel(m_annot_pos.x, m_annot_pos.y, pdfx, pdfy);
				vDraw();
				return true;
			}
			bool OnSelTouchEnd(Point point)
			{
				if (m_status != STA_SELECT) return false;
				CRDVPage* vpage = (CRDVPage*)m_content->vGetPage(m_annot_pos.pageno);
				double pdfx = vpage->GetPDFX(point.X + m_scroller->HorizontalOffset);
				double pdfy = vpage->GetPDFY(point.Y + m_scroller->VerticalOffset);
				m_sel->SetSel(m_annot_pos.x, m_annot_pos.y, pdfx, pdfy);
				vDraw();
				if (m_listener)
					m_listener->OnPDFSelected();
				return true;
			}
			bool m_modified;
			bool m_autofit;
			bool m_touched;
			double m_hold_x;
			double m_hold_y;
			double m_shold_x;
			double m_shold_y;
			PDFRect m_annot_rect;
			PDFPos m_annot_pos;
			PDFAnnot^ m_annot;
			bool OnAnnotTouchBegin(Point point)
			{
				if (m_status != STA_ANNOT) return false;

				m_hold_x = point.X;
				m_hold_y = point.Y;
				m_shold_x = m_hold_x;
				m_shold_y = m_hold_y;
				return true;
			}
			bool OnAnnotTouchMove(Point point)
			{
				if (m_status != STA_ANNOT) return false;
				if (m_doc && m_doc->CanSave)
				{
					m_shold_x = (float)point.X;
					m_shold_y = (float)point.Y;
				}
				vDraw();
				return true;
			}
			bool OnAnnotTouchEnd(Point point)
			{
				if (m_status != STA_ANNOT) return false;

				if (m_doc && m_doc->CanSave)
				{
					m_modified = true;
					float dx = (float)(point.X - m_hold_x);
					float dy = (float)(point.Y - m_hold_y);
					m_annot_rect.left += dx;
					m_annot_rect.top += dy;
					m_annot_rect.right += dx;
					m_annot_rect.bottom += dy;
					PDFPos pos = m_content->vGetPos(point.X, point.Y);
					CRDVPage *vpage = (CRDVPage*)m_content->vGetPage(m_annot_pos.pageno);
					if (pos.pageno == m_annot_pos.pageno)
					{
						PDFMatrix ^mat = vpage->CreateInvertMatrix(m_scroller->HorizontalOffset, m_scroller->VerticalOffset);
						m_annot_rect = mat->TransformRect(m_annot_rect);
						m_annot->Rect = m_annot_rect;
						m_content->vRenderPage((long long)vpage);
						if (m_listener) m_listener->OnPDFPageUpdated(pos.pageno);
					}
					else
					{
						CRDVPage *vdest = (CRDVPage*)m_content->vGetPage(pos.pageno);
						PDFPage ^dpage = m_doc->GetPage(pos.pageno);
						if (dpage)
						{
							PDFMatrix ^mat = vdest->CreateInvertMatrix(m_scroller->HorizontalOffset, m_scroller->VerticalOffset);
							m_annot_rect = mat->TransformRect(m_annot_rect);
							dpage->ObjsStart();
							m_annot->MoveToPage(dpage, m_annot_rect);
							m_content->vRenderPage((long long)vpage);
							m_content->vRenderPage((long long)vdest);
							if (m_listener)
							{
								m_listener->OnPDFPageUpdated(pos.pageno);
								m_listener->OnPDFPageUpdated(m_annot_pos.pageno);
							}
						}
					}
					PDFAnnotEnd();
				}
				return true;
			}
			PDFPoint m_rects[256];
			int m_rects_cnt;
			bool OnEditTextBoxTouchBegin(Point point)
			{
				if (m_status != STA_TEXT_EDIT) return false;
				if (m_rects_cnt >= 256) return true;
				m_hold_x = (float)point.X;
				m_hold_y = (float)point.Y;
				m_rects[m_rects_cnt << 1].x = m_hold_x;
				m_rects[m_rects_cnt << 1].y = m_hold_y;
				m_rects[(m_rects_cnt << 1) + 1].x = m_hold_x;
				m_rects[(m_rects_cnt << 1) + 1].y = m_hold_y;
				m_rects_cnt++;
				return true;
			}
			bool OnEditTextBoxTouchMove(Point point)
			{
				if (m_status != STA_TEXT_EDIT) return false;
				m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
				m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
				vDraw();
				return true;
			}
			bool OnEditTextBoxTouchEnd(Point point)
			{
				if (m_status != STA_TEXT_EDIT) return false;
				m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
				m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
				vDraw();
				//vRectEnd();
				return true;
			}
			bool OnEllipseTouchBegin(Point point)
			{
				if (m_status != STA_ELLIPSE) return false;
				if (m_rects_cnt >= 256) return true;
				m_hold_x = (float)point.X;
				m_hold_y = (float)point.Y;
				m_rects[m_rects_cnt << 1].x = m_hold_x;
				m_rects[m_rects_cnt << 1].y = m_hold_y;
				m_rects[(m_rects_cnt << 1) + 1].x = m_hold_x;
				m_rects[(m_rects_cnt << 1) + 1].y = m_hold_y;
				m_rects_cnt++;
				return true;
			}
			bool OnEllipseTouchMove(Point point)
			{
				if (m_status != STA_ELLIPSE) return false;
				m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
				m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
				vDraw();
				return true;
			}
			bool OnEllipseTouchEnd(Point point)
			{
				if (m_status != STA_ELLIPSE) return false;
				m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
				m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
				vDraw();
				return true;
			}
			bool OnLineTouchBegin(Point point)
			{
				if (m_status != STA_LINE) return false;
				if (m_rects_cnt >= 256) return true;
				m_hold_x = (float)point.X;
				m_hold_y = (float)point.Y;
				m_rects[m_rects_cnt << 1].x = m_hold_x;
				m_rects[m_rects_cnt << 1].y = m_hold_y;
				m_rects[(m_rects_cnt << 1) + 1].x = m_hold_x;
				m_rects[(m_rects_cnt << 1) + 1].y = m_hold_y;
				m_rects_cnt++;
				return true;
			}
			bool OnLineTouchMove(Point point)
			{
				if (m_status != STA_LINE) return false;
				m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
				m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
				vDraw();
				return true;
			}
			bool OnLineTouchEnd(Point point)
			{
				if (m_status != STA_LINE) return false;
				m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
				m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
				vDraw();
				return true;
			}
			PDFInk ^m_ink;
			Path ^m_ink_path;
			int m_ink_pos;
			bool OnInkTouchBegin(Point point)
			{
				if (m_status != STA_INK) return false;
				if (!m_ink)
				{
					//(宽度，颜色)
					m_hold_x = (float)point.X;
					m_hold_y = (float)point.Y;
					m_ink = ref new PDFInk(inkWidth, sm_inkColor);
					m_ink_path = ref new Path();
					m_ink_path->Data = ref new PathGeometry();
					m_ink_pos = 0;
					Windows::UI::Color clr;
					clr.A = (byte)(sm_inkColor >> 24);
					clr.R = (byte)(sm_inkColor >> 16);
					clr.G = (byte)(sm_inkColor >> 8);
					clr.B = (byte)(sm_inkColor);
					m_ink_path->Stroke = ref new SolidColorBrush(clr);
					m_ink_path->StrokeThickness = inkWidth;
					m_ink_path->StrokeStartLineCap = PenLineCap::Round;
					m_ink_path->StrokeLineJoin = PenLineJoin::Round;
				}
				m_ink->Down(point.X, point.Y);
				PathFigure ^inkf = ref new PathFigure();
				((PathGeometry ^)m_ink_path->Data)->Figures->Append(inkf);
				return true;
			}
			bool OnInkTouchMove(Point point)
			{
				if (m_status != STA_INK) return false;
				m_ink->Move(point.X, point.Y);
				vDraw();
				return true;
			}
			bool OnInkTouchEnd(Point point)
			{
				if (m_status != STA_INK) return false;
				m_ink->Up(point.X, point.Y);
				vDraw();
				return true;
			}
			unsigned long long m_tstamp;
			unsigned long long m_tstamp_tap;
			void OnNoneTouchBegin(Point point, unsigned long long timestamp)
			{
				m_tstamp = timestamp;
				m_tstamp_tap = m_tstamp;
				m_hold_x = point.X;
				m_hold_y = point.Y;
				m_shold_x = m_scroller->HorizontalOffset;
				m_shold_y = m_scroller->VerticalOffset;
			}

			void OnNoneTouchMove(Point point, unsigned long long timestamp)
			{
				unsigned long long del = timestamp - m_tstamp;
				if (del > 0)
				{
					double dx = point.X - m_hold_x;
					double dy = point.Y - m_hold_y;
					double vx = dx * 1000000 / del;
					double vy = dy * 1000000 / del;
					dx = 0;
					dy = 0;
					if (vx > 50 || vx < -50)
						dx = vx;
					if (vy > 50 || vy < -50)
						dy = vy;
					else if (timestamp - m_tstamp_tap > 1000000)//long pressed
					{
						dx = point.X - m_hold_x;
						dy = point.Y - m_hold_y;
						if (dx < 10 && dx > -10 && dy < 10 && dy > -10)
						{
							m_status = STA_NONE;
							if (m_listener)
								m_listener->OnPDFLongPressed(point.X, point.Y);
						}
					}
				}
				m_scroller->ChangeView(m_shold_x + m_hold_x - point.X, m_shold_y + m_hold_y - point.Y, m_scroller->ZoomFactor, true);
			}
			struct PDFNoteRec
			{
				CRDVPage *vpage;
				int index;
			};
			PDFNoteRec m_notes[256];
			int m_notes_cnt;
			bool OnNoteTouchBegin(Point point)
			{
				if (m_status != STA_NOTE) return false;
				return true;
			}
			bool OnNoteTouchMove(Point point)
			{
				if (m_status != STA_NOTE) return false;
				return true;
			}
			bool OnNoteTouchEnd(Point point)
			{
				if (m_status != STA_NOTE) return false;
				PDFPos pos = m_content->vGetPos(point.X, point.Y);
				CRDVPage *vpage = (CRDVPage*)m_content->vGetPage(pos.pageno);
				if (vpage)
				{
					PDFPage ^page = m_doc->GetPage(pos.pageno);
					if (page)
					{
						page->ObjsStart();
						if (page->AddAnnotTextNote(pos.x, pos.y))
						{
							m_notes[m_notes_cnt].vpage = vpage;
							m_notes[m_notes_cnt].index = page->AnnotCount - 1;
							m_content->vRenderPage((long long)vpage);
							vDraw();
							if (m_listener)
							{
								page = m_doc->GetPage(pos.pageno);
								page->ObjsStart();
								PDFAnnot ^annot = page->GetAnnot(m_notes[m_notes_cnt].index);
								m_listener->OnPDFAnnotPopup(annot, "", "");
							}
							m_notes_cnt++;
						}
					}
				}
				return true;
			}
			bool OnRectTouchBegin(Point point)
			{
				if (m_status != STA_RECT) return false;
				if (m_rects_cnt >= 256) return true;
				m_hold_x = (float)point.X;
				m_hold_y = (float)point.Y;
				m_rects[m_rects_cnt << 1].x = m_hold_x;
				m_rects[m_rects_cnt << 1].y = m_hold_y;
				m_rects[(m_rects_cnt << 1) + 1].x = m_hold_x;
				m_rects[(m_rects_cnt << 1) + 1].y = m_hold_y;
				m_rects_cnt++;
				return true;
			}
			bool OnRectTouchMove(Point point)
			{
				if (m_status != STA_RECT) return false;
				m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
				m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
				vDraw();
				return true;
			}
			/// <summary>
			/// touch end on rect status.
			/// </summary>
			/// <param name="point"></param>
			/// <returns></returns>
			bool OnRectTouchEnd(Point point)
			{
				if (m_status != STA_RECT) return false;
				m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
				m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
				vDraw();
				//vRectEnd();
				return true;
			}
			void vDrawAnnot()
			{
				if (m_status == STA_ANNOT)
				{
					float dx = (float)(m_shold_x - m_hold_x);
					float dy = (float)(m_shold_y - m_hold_y);
					PDFRect rect = m_annot_rect;
					//PDFRect rect = m_annot.Rect;
					rect.left += dx;
					rect.top += dy;
					rect.right += dx;
					rect.bottom += dy;
					rect.left = to_canvasx(rect.left);
					rect.top = to_canvasy(rect.top);
					rect.right = to_canvasx(rect.right);
					rect.bottom = to_canvasy(rect.bottom);
					m_canvas->draw_rect(rect, 1, 0xFF000000);
				}
			}
			void vDrawInk()
			{
				if (m_status == STA_INK && m_ink)
				{
					m_ink_pos = m_canvas->draw_ink(m_ink, m_ink_path, m_ink_pos, to_canvasx(0), to_canvasy(0));
				}
			}
			void vDrawRects()
			{
				if (m_status == STA_RECT && m_rects_cnt > 0)
				{
					m_canvas->draw_rects(m_rects, m_rects_cnt, sm_rectWidth, sm_rectColor, to_canvasx(0), to_canvasy(0));
				}
			}
			void vDrawTextEditRects()
			{
				if (m_status == STA_TEXT_EDIT && m_rects_cnt > 0)
				{
					m_canvas->draw_rects(m_rects, m_rects_cnt, 2, 0xFF0000FF, to_canvasx(0), to_canvasy(0));
				}
			}
			void vDrawEllipse()
			{
				if (m_status == STA_ELLIPSE && m_rects_cnt > 0)
				{
					m_canvas->draw_ovals(m_rects, m_rects_cnt, sm_ovalWidth, sm_ovalColor, to_canvasx(0), to_canvasy(0));
				}
			}
			void vDrawLines()
			{
				if (m_status == STA_LINE && m_rects_cnt > 0)
				{
					m_canvas->draw_lines(m_rects, m_rects_cnt, sm_lineWidth, sm_lineColor, to_canvasx(0), to_canvasy(0));
				}
			}
			/// <summary>
			/// draw the view.
			/// </summary>
			void vDraw()
			{
				if (!m_content) return;
				m_canvas->clear();
				vDrawAnnot();
				vDrawInk();
				vDrawEllipse();
				vDrawRects();
				vDrawTextEditRects();
				vDrawLines();
				m_content->vDraw(m_scroller->HorizontalOffset, m_scroller->VerticalOffset);
				m_content->vDrawFind(to_contx(m_scroller->HorizontalOffset), to_conty(m_scroller->VerticalOffset));
				if (m_listener)
				{
					int pageno = m_content->vGetPage(m_canvas->ActualWidth * 0.25, m_canvas->ActualHeight * 0.25);
					if (pageno != m_pageno)
					{
						m_pageno = pageno;
						m_listener->OnPDFPageChanged(m_pageno);
					}
					double scale = m_content->vGetScale();
					if (scale != m_scale)
					{
						m_scale = scale;
						m_listener->OnPDFScaleChanged(m_scale);
					}
				}
				if(m_sel) m_sel->DrawSel(m_content, (CRDVPage *)m_content->vGetPage(m_sel->GetPageNo()));
			}

			static float sm_ovalWidth;
			static unsigned int sm_textColor;
			static unsigned int sm_ovalColor;
			static float sm_rectWidth;
			static unsigned int sm_rectColor;
			static float sm_inkWidth;
			static unsigned int sm_inkColor;
			static float sm_lineWidth;
			static unsigned int sm_lineColor;
			static PDF_LAYOUT_MODE sm_viewMode;

			double m_oldZoom;
			int m_pageno;
			double m_scale;
			bool m_keepauto;
			PDFDoc^ m_doc;
			ScrollViewer^ m_scroller;
			PDFVContent^ m_content;
			PDFVCanvas^ m_canvas;//draw some temp elements, over ScrollViewer and layout view.
			RelativePanel^ m_parent;
			PDF_LAYOUT_MODE m_cur_mode;
			IPDFViewListener^ m_listener;

			Windows::Foundation::EventRegistrationToken cookie_pointPress;
			Windows::Foundation::EventRegistrationToken cookie_pointMove;
			Windows::Foundation::EventRegistrationToken cookie_pointUp;
			Windows::Foundation::EventRegistrationToken cookie_pointCancel;
			Windows::Foundation::EventRegistrationToken cookie_pointerExited;
			Windows::Foundation::EventRegistrationToken cookie_tapped;
			Windows::Foundation::EventRegistrationToken cookie_doubleTapped;
			Windows::Foundation::EventRegistrationToken cookie_sizeChanged;
			Windows::Foundation::EventRegistrationToken cookie_viewChanged;
		};
		*/
		public interface class IPDFThumbListener
		{
			void OnPDFPageSelected(int pageno);
		};
		/*
		public ref class PDFThumb sealed : public IPDFContentListener
		{
		public:
			PDFThumb()
			{
				m_parent = nullptr;
				m_scroller = ref new ScrollViewer();
				m_content = ref new PDFVContent();
				m_scroller->Content = m_content;
				m_page_sel = -1;
				m_scale = -1;

				m_doc = nullptr;
				m_sel = nullptr;
				m_touched = false;
			}
			virtual ~PDFThumb()
			{
				PDFClose();
			}
			boolean PDFOpen(RelativePanel^ parent, PDFDoc^ doc, IPDFThumbListener^ listener)
			{
				if (!parent || !doc) return false;
				m_parent = parent;
				m_doc = doc;
				m_listener = listener;
				m_page_sel = -1;
				m_scale = -1;

				Windows::UI::Color clr;
				clr.A = 255;
				clr.R = 224;
				clr.G = 224;
				clr.B = 224;
				m_parent->Background = ref new SolidColorBrush(clr);

				//clr.R = 255;
				//m_content->Background = ref new SolidColorBrush(clr);
				m_parent->Children->Append(m_scroller);
				m_scroller->SetValue(RelativePanel::AlignLeftWithPanelProperty, true);
				m_scroller->SetValue(RelativePanel::AlignTopWithPanelProperty, true);
				m_scroller->SetValue(RelativePanel::AlignRightWithPanelProperty, true);
				m_scroller->SetValue(RelativePanel::AlignBottomWithPanelProperty, true);

				m_content->vOpen(m_doc, (PDF_LAYOUT_MODE)100, this);
				//all coordinate events shall from parent.
				cookie_pointPress = m_parent->PointerPressed += ref new PointerEventHandler(this, &PDFThumb::vOnTouchDown);
				cookie_pointMove = m_parent->PointerMoved += ref new PointerEventHandler(this, &PDFThumb::vOnTouchMove);;
				cookie_pointUp = m_parent->PointerReleased += ref new PointerEventHandler(this, &PDFThumb::vOnTouchUp);;
				cookie_pointCancel = m_parent->PointerCanceled += ref new PointerEventHandler(this, &PDFThumb::vOnTouchUp);;
				cookie_pointerExited = m_parent->PointerExited += ref new PointerEventHandler(this, &PDFThumb::vOnTouchUp);;
				cookie_tapped = m_parent->Tapped += ref new TappedEventHandler(this, &PDFThumb::vOnTapped);

				cookie_sizeChanged = m_scroller->SizeChanged += ref new SizeChangedEventHandler(this, &PDFThumb::vOnSizeChanged);
				cookie_viewChanged = m_scroller->ViewChanged += ref new EventHandler<ScrollViewerViewChangedEventArgs^>(this, &PDFThumb::vOnViewChanged);
				//m_view->LayoutUpdated += OnLayout;
				m_scroller->ZoomMode = ZoomMode::Disabled;
				m_scroller->IsZoomChainingEnabled = false;
				m_scroller->MinZoomFactor = 1;
				m_scroller->MaxZoomFactor = 1;

				m_scroller->HorizontalScrollBarVisibility = ScrollBarVisibility::Visible;
				m_scroller->VerticalScrollBarVisibility = ScrollBarVisibility::Visible;
				m_scroller->IsHoldingEnabled = true;
				m_scroller->IsScrollInertiaEnabled = true;
				m_scroller->IsHitTestVisible = true;

				if (m_scroller->ActualWidth > 0 && m_scroller->ActualHeight)
				{
					m_content->vResize(m_scroller->ActualWidth, m_scroller->ActualHeight, m_scroller->ZoomFactor);
					vDraw();
				}
				return true;
			}
			void PDFSetSelPage(int pageno)
			{
				m_content->vSetPageSel(pageno);
			}
			void PDFUpdatePage(int pageno)
			{
				CRDVPage* vpage = (CRDVPage *)m_content->vGetPage(pageno);
				if (vpage)
				{
					m_content->vRenderPage((long long)vpage);
					vDraw();
				}
			}
			void PDFClose()
			{
				if (!m_doc) return;
				m_scroller->SizeChanged -= cookie_sizeChanged;
				m_scroller->ViewChanged -= cookie_viewChanged;
				if (m_parent)
				{
					m_parent->PointerPressed -= cookie_pointPress;
					m_parent->PointerMoved -= cookie_pointMove;
					m_parent->PointerReleased -= cookie_pointUp;
					m_parent->PointerCanceled -= cookie_pointCancel;
					m_parent->PointerExited -= cookie_pointerExited;
					m_parent->Tapped -= cookie_tapped;
					m_parent->Children->Clear();
					m_parent = nullptr;
				}
				m_sel = nullptr;
				m_scroller->Content = nullptr;
				m_content->vClose();
				m_content = nullptr;
				m_listener = nullptr;
				m_scroller = nullptr;
				m_doc = nullptr;
				m_touched = false;
			}
			void cFillRect(const PDFRect&rect, Windows::UI::Color clr) override//to fill rectangles
			{
				if (!m_sel)
				{
					m_sel = ref new Rectangle();
					m_sel->Fill = ref new SolidColorBrush(clr);
					m_content->Children->Append(m_sel);
				}
				m_sel->SetValue(Canvas::LeftProperty, rect.left);
				m_sel->SetValue(Canvas::TopProperty, rect.top);
				m_sel->Width = rect.right - rect.left;
				m_sel->Height = rect.bottom - rect.top;
			}
			void cSetPos(double vx, double vy) override//to set scroll position.
			{
				m_scroller->ChangeView(vx, vy, (float)m_content->vGetScale(), false);
			}
			void cDetachBmp(WriteableBitmap^ bmp) override
			{
				if (m_listener)
					m_listener->OnPDFDetachBmp(bmp);
			}
			void cAttachBmp(WriteableBitmap^ bmp, const Array<byte>^ arr) override
			{
				if (m_listener)
					m_listener->OnPDFAttachBmp(bmp, arr);
			}
			void cFound(bool found) override//never called
			{
			}
		private:
			//Windows::UI::Xaml::SizeChangedEventHandler
			void vOnSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
			{
				if (!m_content) return;
				m_content->vResize(e->NewSize.Width, e->NewSize.Height, m_scroller->ZoomFactor);
				vDraw();
			}
			void vOnTouchDown(Object^ sender, PointerRoutedEventArgs^ e)
			{
				PointerPoint^ ppt = e->GetCurrentPoint(m_parent);
				//m_parent->CapturePointer(e->Pointer);
				Point pt = ppt->Position;
				pt.X = to_contx(pt.X);
				pt.Y = to_conty(pt.Y);
				m_touched = true;
				OnNoneTouchBegin(pt, ppt->Timestamp);
			}
			void vOnTouchMove(Object^ sender, PointerRoutedEventArgs^ e)
			{
				if (m_touched)
				{
					PointerPoint^ ppt = e->GetCurrentPoint(m_parent);
					Point pt = ppt->Position;
					pt.X = to_contx(pt.X);
					pt.Y = to_conty(pt.Y);
					OnNoneTouchMove(pt, ppt->Timestamp);
				}
			}
			void vOnTouchUp(Object^ sender, PointerRoutedEventArgs^ e)
			{
				if (m_touched)
				{
					//m_parent->ReleasePointerCapture(e->Pointer);
					PointerPoint^ ppt = e->GetCurrentPoint(m_parent);
					Point pt = ppt->Position;
					pt.X = to_contx(pt.X);
					pt.Y = to_conty(pt.Y);
					m_touched = false;
				}
			}
			void vOnTapped(Object^ sender, TappedRoutedEventArgs^ e)
			{
				Point point = e->GetPosition(m_parent);
				point.X = to_contx(point.X);
				point.Y = to_conty(point.Y);
				int pageno = m_content->vGetPage(point.X, point.Y);
				if (m_page_sel != pageno)
				{
					m_page_sel = pageno;
					m_content->vSetPageSel(pageno);
					if (m_listener) m_listener->OnPDFPageSelected(pageno);
				}
			}
			void vOnViewChanged(Object^ sender, ScrollViewerViewChangedEventArgs^ e)
			{
				if (!m_content) return;
				vDraw();
			}

			//map coordinate from ScrollViewer to content View
			inline double to_contx(double x)
			{
				double tdx = m_content->ActualWidth * m_scroller->ZoomFactor;
				if (tdx < m_parent->ActualWidth)
					return x - (m_parent->ActualWidth - tdx) * 0.5;
				else
					return x;
			}
			inline double to_conty(double y)
			{
				double tdy = m_content->ActualHeight * m_scroller->ZoomFactor;
				if (tdy < m_parent->ActualHeight)
					return y - (m_parent->ActualHeight - tdy) * 0.5;
				else
					return y;
			}
			inline double to_canvasx(double x)
			{
				double tdx = m_content->ActualWidth * m_scroller->ZoomFactor;
				if (tdx < m_parent->ActualWidth)
					return x + (m_parent->ActualWidth - tdx) * 0.5;
				else
					return x;
			}
			inline double to_canvasy(double y)
			{
				double tdy = m_content->ActualHeight * m_scroller->ZoomFactor;
				if (tdy < m_parent->ActualHeight)
					return y + (m_parent->ActualHeight - tdy) * 0.5;
				else
					return y;
			}
			bool m_touched;
			double m_hold_x;
			double m_hold_y;
			double m_shold_x;
			double m_shold_y;
			unsigned long long m_tstamp;
			unsigned long long m_tstamp_tap;
			void OnNoneTouchBegin(Point point, unsigned long long timestamp)
			{
				m_tstamp = timestamp;
				m_tstamp_tap = m_tstamp;
				m_hold_x = point.X;
				m_hold_y = point.Y;
				m_shold_x = m_scroller->HorizontalOffset;
				m_shold_y = m_scroller->VerticalOffset;
			}

			void OnNoneTouchMove(Point point, unsigned long long timestamp)
			{
				m_scroller->ChangeView(m_shold_x + m_hold_x - point.X, m_shold_y + m_hold_y - point.Y, m_scroller->ZoomFactor, true);
			}
			/// <summary>
			/// draw the view.
			/// </summary>
			void vDraw()
			{
				if (!m_content) return;
				m_content->vDraw(m_scroller->HorizontalOffset, m_scroller->VerticalOffset);
				if (m_sel)
				{
					unsigned int idx = -1;
					if (m_content->Children->IndexOf(m_sel, &idx))
						m_content->Children->Move(idx, m_content->Children->Size - 1);
				}
			}

			int m_page_sel;
			double m_scale;
			PDFDoc^ m_doc;
			ScrollViewer^ m_scroller;
			PDFVContent^ m_content;
			RelativePanel^ m_parent;
			Rectangle^ m_sel;
			IPDFThumbListener^ m_listener;

			Windows::Foundation::EventRegistrationToken cookie_pointPress;
			Windows::Foundation::EventRegistrationToken cookie_pointMove;
			Windows::Foundation::EventRegistrationToken cookie_pointUp;
			Windows::Foundation::EventRegistrationToken cookie_pointCancel;
			Windows::Foundation::EventRegistrationToken cookie_pointerExited;
			Windows::Foundation::EventRegistrationToken cookie_tapped;
			Windows::Foundation::EventRegistrationToken cookie_sizeChanged;
			Windows::Foundation::EventRegistrationToken cookie_viewChanged;
		};
		*/
	}
}