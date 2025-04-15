#include "pch.h"
#include "DXView.h"
#include "PDFCore.h"
#include "DXLayout.h"
#include "DXBlock.h"
using namespace RDPDFLib::pdf;
using namespace RDPDFLib::view;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

DXView::DXView(Windows::UI::Xaml::Controls::SwapChainPanel^ panel, Windows::UI::Xaml::Controls::Canvas ^canvas)
	:m_device(panel, this)
{
	m_page_gap = 4;
	m_device.StartRenderLoop();
	m_canvas = canvas;
	CoreWindow^ window = Window::Current->CoreWindow;
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
	window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DXView::dx_visibility_changed);
	currentDisplayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DXView::dx_dpi_changed);
	currentDisplayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DXView::dx_orientation_changed);
	DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DXView::dx_invalidate);

	panel->CompositionScaleChanged += ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DXView::onVCompositionScaleChanged);
	panel->SizeChanged += ref new SizeChangedEventHandler(this, &DXView::onVSizeChanged);

	m_layout = NULL;
	m_status = STA_NONE;
	m_annot_page = NULL;
	m_annot_pos.pageno = -1;
	m_annot_rect.left = 0;
	m_annot_rect.top = 0;
	m_annot_rect.right = 0;
	m_annot_rect.bottom = 0;
	m_notes = NULL;
	m_notes_cnt = 0;
	m_sel = NULL;
	m_ink = nullptr;
	m_rects = NULL;
	m_rects_cnt = 0;

	panel->PointerPressed += ref new PointerEventHandler(this, &DXView::onVTouchDown);
	panel->PointerMoved += ref new PointerEventHandler(this, &DXView::onVTouchMove);
	panel->PointerReleased += ref new PointerEventHandler(this, &DXView::onVTouchUp);
	panel->PointerCanceled += ref new PointerEventHandler(this, &DXView::onVTouchUp);
	panel->PointerExited += ref new PointerEventHandler(this, &DXView::onVTouchUp);
	panel->PointerWheelChanged += ref new PointerEventHandler(this, &DXView::onVWheelChanged);
	panel->Tapped += ref new TappedEventHandler(this, &DXView::onVTapped);
	panel->DoubleTapped += ref new DoubleTappedEventHandler(this, &DXView::onVDoubleTapped);
	panel->ManipulationMode = ManipulationModes::Scale + ManipulationModes::TranslateX + ManipulationModes::TranslateY + ManipulationModes::TranslateInertia;
	panel->ManipulationStarted += ref new ManipulationStartedEventHandler(this, &DXView::onVManipulationStarted);
	panel->ManipulationDelta += ref new ManipulationDeltaEventHandler(this, &DXView::onVManipulationDelta);
	panel->ManipulationCompleted += ref new ManipulationCompletedEventHandler(this, &DXView::onVManipulationCompleted);
	panel->ManipulationInertiaStarting += ref new ManipulationInertiaStartingEventHandler(this, &DXView::onVManipulationInertiaStarting);
}

void DXView::dx_visibility_changed(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	if (args->Visible)
	{
		m_device.StartRenderLoop();
	}
	else
	{
		m_device.StopRenderLoop();
	}
}

void DXView::dx_dpi_changed(DisplayInformation^ sender, Object^ args)
{
	m_device.SetDpi(sender->LogicalDpi);
}

void DXView::dx_orientation_changed(DisplayInformation^ sender, Object^ args)
{
	m_device.SetCurrentOrientation(sender->CurrentOrientation);
}

void DXView::dx_invalidate(DisplayInformation^ sender, Object^ args)
{
	m_device.ValidateDevice();
}

void DXView::dx_update()
{
}

bool DXView::dx_render()
{
	if(m_layout) m_layout->vDraw();
	return true;
}

void DXView::onVCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	m_device.SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
}

void DXView::onVSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	m_device.SetLogicalSize(e->NewSize);
	if (m_layout) m_layout->vResize(e->NewSize.Width, e->NewSize.Height);
}

void DXView::onVPageChanged(int pageno)
{
	if (m_listener) m_listener->OnPDFPageChanged(pageno);
}

void DXView::onVFound(bool found)
{
	if (m_listener) m_listener->OnPDFSearchFinished(found);
}

void DXView::onVCacheRendered(int pageno)
{
	if (m_listener) m_listener->OnPDFPageRendered(pageno);
}

bool DXView::staNoneDrawCanvas()
{
	if (m_status != STA_NONE) return false;
	return true;
}

bool DXView::staNoneTouchDown(float x, float y)
{
	if (m_status != STA_NONE) return false;
	m_layout->vEndScroll();
	m_hold_x = x;
	m_hold_y = y;
	m_hold_docx = m_layout->vGetX();
	m_hold_docy = m_layout->vGetY();
	return true;
}
bool DXView::staNoneTouchMove(float x, float y)
{
	if (m_status != STA_NONE) return false;
	m_layout->vSetX((int)(m_hold_docx + m_hold_x - x));
	m_layout->vSetY((int)(m_hold_docy + m_hold_y - y));
	return true;
}
bool DXView::staNoneTouchUp(float x, float y)
{
	if (m_status != STA_NONE) return false;
	m_layout->vSetX((int)(m_hold_docx + m_hold_x - x));
	m_layout->vSetY((int)(m_hold_docy + m_hold_y - y));
	m_layout->vMoveEnd();
	return true;
}

bool DXView::staSelectDrawCanvas()
{
	if (m_status != STA_SELECT) return false;
	if (m_sel && m_annot_page)
	{
		int orgx = m_annot_page->GetVX(0) - m_layout->vGetX();
		int orgy = m_annot_page->GetVY(m_doc->GetPageHeight(m_annot_page->GetPageNo())) - m_layout->vGetY();
		float scale = m_annot_page->GetScale();
		float pheight = m_doc->GetPageHeight(m_annot_page->GetPageNo());
		m_sel->DrawSel(m_canvas, scale, pheight, orgx, orgy);
		PDFRect rect1;
		m_sel->GetRect1(scale, pheight, orgx, orgy, rect1);
		PDFRect rect2;
		m_sel->GetRect2(scale, pheight, orgx, orgy, rect2);
		if (rect1.left < rect1.right && rect2.left < rect2.right)
		{
			//radaee: draw icon for selection.
			//canvas.drawBitmap(m_sel_icon1, rect1[0] - m_sel_icon1.getWidth(), rect1[1] - m_sel_icon1.getHeight(), null);
			//canvas.drawBitmap(m_sel_icon2, rect2[2], rect2[3], null);
		}
	}
	return true;
}

bool DXView::staSelectTouchDown(float x, float y)
{
	if (m_status != STA_SELECT) return false;
	m_hold_x = x;
	m_hold_y = y;
	if (m_sel)
	{
		m_sel->Clear();
		delete m_sel;
	}
	m_layout->vGetPos((int)m_hold_x, (int)m_hold_y, m_annot_pos);
	m_annot_page = m_layout->vGetPage(m_annot_pos.pageno);
	m_sel = new DXSel(m_doc->GetPage(m_annot_pos.pageno));
	return true;
}

bool DXView::staSelectTouchMove(float x, float y)
{
	if (m_status != STA_SELECT) return false;
	if (m_sel && m_annot_pos.pageno >= 0 && m_annot_page && m_layout)
	{
		m_sel->SetSel(m_annot_pos.x, m_annot_pos.y, m_annot_page->ToPDFX(x, m_layout->vGetX()), m_annot_page->ToPDFY(y, m_layout->vGetY()));
		onVDrawCanvas();
	}
	return true;
}

bool DXView::staSelectTouchUp(float x, float y)
{
	if (m_status != STA_SELECT) return false;
	if (m_sel && m_annot_pos.pageno >= 0 && m_annot_page && m_layout)
	{
		m_sel->SetSel(m_annot_pos.x, m_annot_pos.y, m_annot_page->ToPDFX(x, m_layout->vGetX()), m_annot_page->ToPDFY(y, m_layout->vGetY()));
		onVDrawCanvas();
		if (m_listener) m_listener->OnPDFSelectEnd(m_sel->GetSelString());
	}
	return true;
}

bool DXView::staAnnotDrawCanvas()
{
	if (m_status != STA_ANNOT) return false;
	if (m_status == STA_ANNOT)
	{
		//radaee: color and width is fixed below:
		Color clr;
		clr.A = 128;
		clr.B = 0;
		clr.G = 0;
		clr.R = 0;
		DXCanvas::draw_rect(m_canvas, clr, m_annot_rect.left, m_annot_rect.top, m_annot_rect.right, m_annot_rect.bottom, 1);
	}
	return true;
}

bool DXView::staAnnotTouchDown(float x, float y)
{
	if (m_status != STA_ANNOT) return false;
	m_annot_x0 = x;
	m_annot_y0 = y;
	if (m_annot_x0 > m_annot_rect.left && m_annot_y0 > m_annot_rect.top &&
		m_annot_x0 < m_annot_rect.right && m_annot_y0 < m_annot_rect.bottom) {
		m_annot_rect0 = m_annot_rect;
	}
	else
	{
		m_annot_rect0.left = 0;
		m_annot_rect0.top = 0;
		m_annot_rect0.right = 0;
		m_annot_rect0.bottom = 0;
	}
	onVDrawCanvas();
	return true;
}

bool DXView::staAnnotTouchMove(float x, float y)
{
	if (m_status != STA_ANNOT) return false;
	if (m_annot_rect0.left < m_annot_rect0.right)
	{
		m_annot_rect.left = m_annot_rect0.left + x - m_annot_x0;
		m_annot_rect.top = m_annot_rect0.top + y - m_annot_y0;
		m_annot_rect.right = m_annot_rect0.right + x - m_annot_x0;
		m_annot_rect.bottom = m_annot_rect0.bottom + y - m_annot_y0;
	}
	onVDrawCanvas();
	return true;
}

bool DXView::staAnnotTouchUp(float x, float y)
{
	if (m_status != STA_ANNOT) return false;
	if (m_annot_rect0.left < m_annot_rect0.right)
	{
		PDFPos pos;
		m_layout->vGetPos(x, y, pos);
		m_annot_rect.left = m_annot_rect0.left + x - m_annot_x0;
		m_annot_rect.top = m_annot_rect0.top + y - m_annot_y0;
		m_annot_rect.right = m_annot_rect0.right + x - m_annot_x0;
		m_annot_rect.bottom = m_annot_rect0.bottom + y - m_annot_y0;
		if (m_annot_page->GetPageNo() == pos.pageno)
		{
			m_annot_rect0.left = m_annot_page->ToPDFX(m_annot_rect.left, m_layout->vGetX());
			m_annot_rect0.top = m_annot_page->ToPDFY(m_annot_rect.bottom, m_layout->vGetY());
			m_annot_rect0.right = m_annot_page->ToPDFX(m_annot_rect.right, m_layout->vGetX());
			m_annot_rect0.bottom = m_annot_page->ToPDFY(m_annot_rect.top, m_layout->vGetY());
			//add to redo/undo stack.
			PDFRect rect = m_annot->Rect;
			m_opstack.push(new OPMove(pos.pageno, rect, pos.pageno, m_annot->IndexInPage, m_annot_rect0));
			m_annot->Rect = m_annot_rect0;
			m_layout->vRenderPage(m_annot_page);
			if (m_listener) m_listener->OnPDFPageModified(m_annot_page->GetPageNo());
		}
		else
		{
			DXPage *vpage = m_layout->vGetPage(pos.pageno);
			PDFPage ^page = m_doc->GetPage(vpage->GetPageNo());
			if (page)
			{
				page->ObjsStart();
				m_annot_rect0.left = vpage->ToPDFX(m_annot_rect.left, m_layout->vGetX());
				m_annot_rect0.top = vpage->ToPDFY(m_annot_rect.bottom, m_layout->vGetY());
				m_annot_rect0.right = vpage->ToPDFX(m_annot_rect.right, m_layout->vGetX());
				m_annot_rect0.bottom = vpage->ToPDFY(m_annot_rect.top, m_layout->vGetY());
				//add to redo/undo stack.
				PDFRect rect = m_annot->Rect;
				m_opstack.push(new OPMove(m_annot_page->GetPageNo(), rect, pos.pageno, page->AnnotCount, m_annot_rect0));
				m_annot->MoveToPage(page, m_annot_rect0);
				//page.CopyAnnot(m_annot, m_annot_rect0);
				page->Close();
			}
			m_layout->vRenderPage(m_annot_page);
			m_layout->vRenderPage(vpage);
			if (m_listener) {
				m_listener->OnPDFPageModified(m_annot_page->GetPageNo());
				m_listener->OnPDFPageModified(vpage->GetPageNo());
			}
		}
	}
	PDFEndAnnot();
	onVDrawCanvas();
	return true;
}

bool DXView::staNoteDrawCanvas()
{
	if (m_status != STA_NOTE) return false;
	return true;
}

bool DXView::staNoteTouchDown(float x, float y)
{
	if (m_status != STA_NOTE) return false;
	return true;
}

bool DXView::staNoteTouchMove(float x, float y)
{
	if (m_status != STA_NOTE) return false;
	return true;
}

bool DXView::staNoteTouchUp(float x, float y)
{
	if (m_status != STA_NOTE) return false;
	PDFPos pos;
	m_layout->vGetPos(x, y, pos);
	DXPage *vpage = m_layout->vGetPage(pos.pageno);
	PDFPage ^page = m_doc->GetPage(vpage->GetPageNo());
	if (page)
	{
		page->ObjsStart();
		if (!m_notes)
		{
			m_notes = (_DXNOTE *)malloc(sizeof(_DXNOTE));
			m_notes->page = vpage;
			m_notes->index = page->AnnotCount;
			m_notes_cnt = 1;
		}
		else
		{
			int cur = 0;
			int cnt = m_notes_cnt;
			while (cur < cnt) {
				if (m_notes[cur].page == vpage) break;
				cur++;
			}
			if (cur >= cnt)//append 1 page
			{
				m_notes_cnt++;
				m_notes = (_DXNOTE *)realloc(m_notes, sizeof(_DXNOTE) * m_notes_cnt);
				m_notes[cnt].page = vpage;
				m_notes[cnt].index = page->AnnotCount;
			}
		}
		page->AddAnnotTextNote(pos.x, pos.y);
		//add to redo/undo stack.
		m_opstack.push(new OPAdd(pos.pageno, page, page->AnnotCount - 1));
		m_layout->vRenderPage(vpage);
		page->Close();
		onVDrawCanvas();

		if (m_listener)
			m_listener->OnPDFPageModified(vpage->GetPageNo());
	}
	return true;
}

bool DXView::staInkDrawCanvas()
{
	if (m_status != STA_INK) return false;
	int cur = 0;
	int cnt = m_ink->NodesCnt;
	PDFPoint pt;
	PDFPoint pt_prev;
	PathGeometry ^inkg = ref new PathGeometry();
	PathFigure ^inkf = nullptr;
	LineSegment ^line = nullptr;
	Windows::UI::Xaml::Media::BezierSegment ^bezier;
	Point ppt;
	Path ^path = ref new Path();
	for (cur = 0; cur < cnt; cur++)
	{
		switch (m_ink->GetOP(cur))
		{
		case 0:
			pt = m_ink->GetPoint(cur);
			if (inkf)
				inkg->Figures->Append(inkf);
			inkf = ref new PathFigure();
			ppt.X = pt.x;
			ppt.Y = pt.y;
			inkf->StartPoint = ppt;
			pt_prev = pt;
			break;
		case 1:
			pt = m_ink->GetPoint(cur);
			line = ref new LineSegment();
			ppt.X = pt.x;
			ppt.Y = pt.y;
			line->Point = ppt;
			inkf->Segments->Append(line);
			break;
		case 2:
			pt = m_ink->GetPoint(cur);
			bezier = ref new Windows::UI::Xaml::Media::BezierSegment();
			ppt.X = pt.x;
			ppt.Y = pt.y;
			bezier->Point1 = ppt;
			bezier->Point2 = ppt;
			pt = m_ink->GetPoint(cur + 1);
			ppt.X = pt.x;
			ppt.Y = pt.y;
			bezier->Point3 = ppt;
			cur++;
			inkf->Segments->Append(bezier);
			break;
		}
	}
	if (inkf)
	{
		inkg->Figures->Append(inkf);
		inkf = nullptr;
	}
	path->Data = inkg;
	Color clr;
	clr.A = 255;
	clr.R = 255;
	clr.G = 0;
	clr.B = 0;
	path->Stroke = ref new SolidColorBrush(clr);
	path->StrokeThickness = 3;
	path->StrokeStartLineCap = PenLineCap::Round;
	path->StrokeLineJoin = PenLineJoin::Round;
	m_canvas->Children->Append(path);
	return true;
}

bool DXView::staInkTouchDown(float x, float y)
{
	if (m_status != STA_INK) return false;
	if (!m_annot_page)
	{
		PDFPos pos;
		m_layout->vGetPos(x, y, pos);
		m_annot_page = m_layout->vGetPage(pos.pageno);
	}
	m_ink->Down(x, y);
	onVDrawCanvas();
	return true;
}

bool DXView::staInkTouchMove(float x, float y)
{
	if (m_status != STA_INK) return false;
	m_ink->Move(x, y);
	onVDrawCanvas();
	return true;
}

bool DXView::staInkTouchUp(float x, float y)
{
	if (m_status != STA_INK) return false;
	m_ink->Up(x, y);
	onVDrawCanvas();
	return true;
}

bool DXView::staLineDrawCanvas()
{
	if (m_status != STA_LINE) return false;
	Color clr;
	clr.A = 255;
	clr.B = 0;
	clr.G = 0;
	clr.R = 255;
	DXCanvas::draw_lines(m_canvas, clr, m_rects, m_rects_cnt, 3);
	return true;
}

bool DXView::staLineTouchDown(float x, float y)
{
	if (m_status != STA_LINE) return false;
	int cur = m_rects_cnt;
	m_rects_cnt += 4;
	m_rects = (float *)realloc(m_rects, sizeof(float) * m_rects_cnt);
	m_rects[cur + 0] = x;
	m_rects[cur + 1] = y;
	m_rects[cur + 2] = x;
	m_rects[cur + 3] = y;
	onVDrawCanvas();
	return true;
}

bool DXView::staLineTouchMove(float x, float y)
{
	if (m_status != STA_LINE) return false;
	m_rects[m_rects_cnt - 2] = x;
	m_rects[m_rects_cnt - 1] = y;
	onVDrawCanvas();
	return true;
}

bool DXView::staLineTouchUp(float x, float y)
{
	if (m_status != STA_LINE) return false;
	m_rects[m_rects_cnt - 2] = x;
	m_rects[m_rects_cnt - 1] = y;
	onVDrawCanvas();
	return true;
}

bool DXView::staRectDrawCanvas()
{
	if (m_status != STA_RECT) return false;
	Color clr;
	clr.A = 255;
	clr.R = 0;
	clr.G = 0;
	clr.B = 255;
	SolidColorBrush ^br = ref new SolidColorBrush(clr);
	float *cur = m_rects;
	float *end = cur + m_rects_cnt;
	while(cur < end)
	{
		Rectangle ^rect = ref new Rectangle();
		rect->StrokeThickness = 3;
		rect->Stroke = br;
		if (cur[0] > cur[2])
		{
			rect->SetValue(Canvas::LeftProperty, cur[2]);
			rect->Width = cur[0] - cur[2];
		}
		else
		{
			rect->SetValue(Canvas::LeftProperty, cur[0]);
			rect->Width = cur[2] - cur[0];
		}
		if (cur[1] > cur[3])
		{
			rect->SetValue(Canvas::TopProperty, cur[3]);
			rect->Height = cur[1] - cur[3];
		}
		else
		{
			rect->SetValue(Canvas::TopProperty, cur[1]);
			rect->Height = cur[3] - cur[1];
		}
		m_canvas->Children->Append(rect);
		cur += 4;
	}
	return true;
}

bool DXView::staRectTouchDown(float x, float y)
{
	if (m_status != STA_RECT) return false;
	int cur = m_rects_cnt;
	m_rects_cnt += 4;
	m_rects = (float *)realloc(m_rects, sizeof(float) * m_rects_cnt);
	m_rects[cur + 0] = x;
	m_rects[cur + 1] = y;
	m_rects[cur + 2] = x;
	m_rects[cur + 3] = y;
	onVDrawCanvas();
	return true;
}

bool DXView::staRectTouchMove(float x, float y)
{
	if (m_status != STA_RECT) return false;
	m_rects[m_rects_cnt - 2] = x;
	m_rects[m_rects_cnt - 1] = y;
	onVDrawCanvas();
	return true;
}

bool DXView::staRectTouchUp(float x, float y)
{
	if (m_status != STA_RECT) return false;
	m_rects[m_rects_cnt - 2] = x;
	m_rects[m_rects_cnt - 1] = y;
	onVDrawCanvas();
	return true;
}

bool DXView::staEllipseDrawCanvas()
{
	if (m_status != STA_ELLIPSE) return false;
	Color clr;
	clr.A = 255;
	clr.R = 0;
	clr.G = 0;
	clr.B = 255;
	SolidColorBrush ^br = ref new SolidColorBrush(clr);
	float *cur = m_rects;
	float *end = cur + m_rects_cnt;
	while (cur < end)
	{
		Windows::UI::Xaml::Shapes::Ellipse ^rect = ref new Windows::UI::Xaml::Shapes::Ellipse();
		rect->StrokeThickness = 3;
		rect->Stroke = br;
		if (cur[0] > cur[2])
		{
			rect->SetValue(Canvas::LeftProperty, cur[2]);
			rect->Width = cur[0] - cur[2];
		}
		else
		{
			rect->SetValue(Canvas::LeftProperty, cur[0]);
			rect->Width = cur[2] - cur[0];
		}
		if (cur[1] > cur[3])
		{
			rect->SetValue(Canvas::TopProperty, cur[3]);
			rect->Height = cur[1] - cur[3];
		}
		else
		{
			rect->SetValue(Canvas::TopProperty, cur[1]);
			rect->Height = cur[3] - cur[1];
		}
		m_canvas->Children->Append(rect);
		cur += 4;
	}

	return true;
}

bool DXView::staStampDrawCanvas()
{
	if (m_status != STA_STAMP) return false;
	return true;
}

bool DXView::staStampTouchDown(float x, float y)
{
	if (m_status != STA_STAMP) return false;
	return true;
}

bool DXView::staStampTouchMove(float x, float y)
{
	if (m_status != STA_STAMP) return false;
	return true;
}

bool DXView::staStampTouchUp(float x, float y)
{
	if (m_status != STA_STAMP) return false;
	return true;
}

bool DXView::staEllipseTouchDown(float x, float y)
{
	if (m_status != STA_ELLIPSE) return false;
	int cur = m_rects_cnt;
	m_rects_cnt += 4;
	m_rects = (float *)realloc(m_rects, sizeof(float) * m_rects_cnt);
	m_rects[cur + 0] = x;
	m_rects[cur + 1] = y;
	m_rects[cur + 2] = x;
	m_rects[cur + 3] = y;
	onVDrawCanvas();
	return true;
}

bool DXView::staEllipseTouchMove(float x, float y)
{
	if (m_status != STA_ELLIPSE) return false;
	m_rects[m_rects_cnt - 2] = x;
	m_rects[m_rects_cnt - 1] = y;
	onVDrawCanvas();
	return true;
}

bool DXView::staEllipseTouchUp(float x, float y)
{
	if (m_status != STA_ELLIPSE) return false;
	m_rects[m_rects_cnt - 2] = x;
	m_rects[m_rects_cnt - 1] = y;
	onVDrawCanvas();
	return true;
}


void DXView::onVTapped(Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs ^e)
{
	if (m_status != STA_NONE && m_status != STA_ANNOT) return;
	Point pt = e->GetPosition(m_device.GetPanel());
	m_layout->vGetPos(pt.X, pt.Y, m_annot_pos);
	m_annot_page = m_layout->vGetPage(m_annot_pos.pageno);
	PDFPage ^page = m_doc->GetPage(m_annot_page->GetPageNo());
	if (!page) m_annot = nullptr;
	else m_annot = page->GetAnnot(m_annot_pos.x, m_annot_pos.y);
	if (!m_annot)
	{
		m_annot_page = NULL;
		m_annot_rect.left = 0;
		m_annot_rect.top = 0;
		m_annot_rect.right = 0;
		m_annot_rect.bottom = 0;
		if (m_listener)
		{
			if (m_status == STA_ANNOT)
				m_listener->OnPDFAnnotTapped(m_annot_pos.pageno, nullptr);
			else
				m_listener->OnPDFBlankTapped();
		}
		m_annot_pos.pageno = -1;
		m_status = STA_NONE;
	}
	else
	{
		page->ObjsStart();
		m_annot_rect = m_annot->Rect;
		float tmp = m_annot_rect.top;
		m_annot_rect.left = m_annot_page->GetVX(m_annot_rect.left) - m_layout->vGetX();
		m_annot_rect.top = m_annot_page->GetVY(m_annot_rect.bottom) - m_layout->vGetY();
		m_annot_rect.right = m_annot_page->GetVX(m_annot_rect.right) - m_layout->vGetX();
		m_annot_rect.bottom = m_annot_page->GetVY(tmp) - m_layout->vGetY();
		m_status = STA_ANNOT;
		int check = m_annot->GetCheckStatus();
		if (m_doc->CanSave && check >= 0)
		{
			switch (check) {
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
			m_layout->vRenderPage(m_annot_page);
			if (m_listener)
				m_listener->OnPDFPageModified(m_annot_page->GetPageNo());
			PDFEndAnnot();
		}
		else if (m_doc->CanSave && m_annot->EditType > 0)//if form edit-box.
		{
			//radaee: show editbox
		}
		else if (m_doc->CanSave && m_annot->ComboItemCount >= 0)//if form choice
		{
			//radaee: show droplist
		}
		else if (m_listener)
			m_listener->OnPDFAnnotTapped(m_annot_pos.pageno, m_annot);
		onVDrawCanvas();
	}
}

void DXView::onVDoubleTapped(Object^ sender, Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs ^e)
{
	if (m_status != STA_NONE) return;
	Point pt = e->GetPosition(m_device.GetPanel());
	if (m_listener) m_listener->OnPDFDoubleTapped(pt.X, pt.Y);
}

void DXView::onVManipulationStarted(Object ^sender, Windows::UI::Xaml::Input::ManipulationStartedRoutedEventArgs ^ e)
{
}

void DXView::onVManipulationDelta(Object ^sender, Windows::UI::Xaml::Input::ManipulationDeltaRoutedEventArgs ^ e)
{
	if (!m_layout || m_pts.m_pts_cnt < 2) return;
	if (m_status == STA_NONE)
	{
		m_zoom_scale = m_layout->vGetZoom();
		m_hold_x = e->Position.X;
		m_hold_y = e->Position.Y;
		m_layout->vGetPos(m_hold_x, m_hold_y, m_annot_pos);
		m_layout->vZoomStart();
		m_status = STA_ZOOM;
		if (m_listener) m_listener->OnPDFZoomStart();
	}
	else if (m_status == STA_ZOOM)
	{
		m_layout->vZoomSet(m_zoom_scale * e->Cumulative.Scale);
		m_layout->vSetPos(m_hold_x, m_hold_y, m_annot_pos);
	}
}

void DXView::onVManipulationCompleted(Object ^sender, Windows::UI::Xaml::Input::ManipulationCompletedRoutedEventArgs ^ e)
{
	if (m_status == STA_ZOOM)
	{
		m_layout->vZoomConfirm();
		m_status = STA_NONE;
		if (m_listener) m_listener->OnPDFZoomEnd();
	}
}

void DXView::onVManipulationInertiaStarting(Object ^sender, Windows::UI::Xaml::Input::ManipulationInertiaStartingRoutedEventArgs ^e)
{
	if (m_status != STA_NONE || !m_pts.m_hold) return;
	m_pts.m_hold = false;
	float vx = e->Velocities.Linear.X;
	float vy = e->Velocities.Linear.Y;
	m_layout->vFling(vx * 1000, vy * 1000);
}

void DXView::onVDrawCanvas()
{
	m_canvas->Children->Clear();
	if (!m_layout) return;
	m_layout->vFindDraw();
	if (staSelectDrawCanvas()) return;
	if (staRectDrawCanvas()) return;
	if (staEllipseDrawCanvas()) return;
	if (staAnnotDrawCanvas()) return;
	if (staLineDrawCanvas()) return;
	if (staInkDrawCanvas()) return;
	if (staStampDrawCanvas()) return;
	if (staNoteDrawCanvas()) return;
	staNoneDrawCanvas();
}

void DXView::onVTouchDown(Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	PointerPoint ^ppt = e->GetCurrentPoint(m_device.GetPanel());
	if (!m_pts.find(ppt->PointerId)) m_pts.add(ppt->PointerId);
	if (!m_pts.m_hold || !m_layout) return;
	Point pt = ppt->Position;
	if (staSelectTouchDown(pt.X, pt.Y)) return;
	if (staInkTouchDown(pt.X, pt.Y)) return;
	if (staRectTouchDown(pt.X, pt.Y)) return;
	if (staEllipseTouchDown(pt.X, pt.Y)) return;
	if (staNoteTouchDown(pt.X, pt.Y)) return;
	if (staLineTouchDown(pt.X, pt.Y)) return;
	if (staStampTouchDown(pt.X, pt.Y)) return;
	if (staAnnotTouchDown(pt.X, pt.Y)) return;
	staNoneTouchDown(pt.X, pt.Y);
}

void DXView::onVTouchMove(Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	if (!m_pts.m_hold || !m_layout) return;
	PointerPoint ^ppt = e->GetCurrentPoint(m_device.GetPanel());
	Point pt = ppt->Position;
	if (staSelectTouchMove(pt.X, pt.Y)) return;
	if (staInkTouchMove(pt.X, pt.Y)) return;
	if (staRectTouchMove(pt.X, pt.Y)) return;
	if (staEllipseTouchMove(pt.X, pt.Y)) return;
	if (staNoteTouchMove(pt.X, pt.Y)) return;
	if (staLineTouchMove(pt.X, pt.Y)) return;
	if (staStampTouchMove(pt.X, pt.Y)) return;
	if (staAnnotTouchMove(pt.X, pt.Y)) return;
	staNoneTouchMove(pt.X, pt.Y);
}

void DXView::onVTouchUp(Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	PointerPoint ^ppt = e->GetCurrentPoint(m_device.GetPanel());
	bool hold = m_pts.m_hold;
	m_pts.remove(ppt->PointerId);
	if (!hold || !m_layout) return;
	Point pt = ppt->Position;
	if (staSelectTouchUp(pt.X, pt.Y)) return;
	if (staInkTouchUp(pt.X, pt.Y)) return;
	if (staRectTouchUp(pt.X, pt.Y)) return;
	if (staEllipseTouchUp(pt.X, pt.Y)) return;
	if (staNoteTouchUp(pt.X, pt.Y)) return;
	if (staLineTouchUp(pt.X, pt.Y)) return;
	if (staStampTouchUp(pt.X, pt.Y)) return;
	if (staAnnotTouchUp(pt.X, pt.Y)) return;
	staNoneTouchUp(pt.X, pt.Y);
}

void DXView::onVWheelChanged(Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	if (m_status != STA_NONE) return;
	PointerPoint ^ppt = e->GetCurrentPoint(m_device.GetPanel());
	if (m_layout)
	{
		Windows::UI::Core::CoreVirtualKeyStates state = Window::Current->CoreWindow->GetKeyState(Windows::System::VirtualKey::Control);
		if (state == CoreVirtualKeyStates::Down)
		{
			m_layout->vZoomStart();
			if(ppt->Properties->MouseWheelDelta > 0)
				m_layout->vZoomSet(m_layout->vGetZoom() * 1.1f);
			else
				m_layout->vZoomSet(m_layout->vGetZoom() * 0.9f);
			m_layout->vZoomConfirm();
		}
		else m_layout->vWheel(ppt->Properties->MouseWheelDelta * 2);
	}
}

void DXView::PDFOpen(PDFDoc ^doc, IDXViewCallback ^callback, int page_gap)
{
	m_doc = doc;
	m_listener = callback;
	m_page_gap = (page_gap + 1) & -2;
	m_status = STA_NONE;
	PDFSetView(1);
}

void DXView::PDFSetView(int view_mode)
{
	DXLayout *layout;
	PDFPos pos;
	pos.pageno = -1;
	pos.x = 0;
	pos.y = 0;
	Windows::Foundation::Size size = m_device.GetLogicalSize();
	if (m_layout) m_layout->vGetPos(size.Width * 0.5f, size.Height * 0.5f, pos);
	switch (view_mode)
	{
	case 3://single
		layout = new DXLayoutDual(m_canvas, this, DXLayoutDual::ALIGN_CENTER, DXLayoutDual::SCALE_SAME_HEIGHT, false, NULL, 0, NULL, 0);
		break;
	case 4:
	case 6://dual when landscape
	{
		int pcnt = m_doc->PageCount;
		bool *bval = new bool[pcnt];
		bval[0] = false;
		int position = 1;
		for (int pcur = 1; pcur < pcnt; pcur++)
		{
			float pw = m_doc->GetPageWidth(pcur);
			float ph = m_doc->GetPageHeight(pcur);
			if (pw / ph > 1) bval[position] = false;
			else
			{
				float pw_next = m_doc->GetPageWidth(pcur + 1);
				float ph_next = m_doc->GetPageHeight(pcur + 1);
				if (pw_next / ph_next > 1)
					bval[position] = false;
				else
				{
					bval[position] = true;
					pcur++;
				}
			}
			position++;
		}
		layout = new DXLayoutDual(m_canvas, this, DXLayoutDual::ALIGN_CENTER, DXLayoutDual::SCALE_FIT, false, bval, pcnt, NULL, 0);
	}
	break;
	default://vertical.
		layout = new DXLayoutVert(m_canvas, this, DXLayoutVert::ALIGN_CENTER, false);
		break;
	}
	layout->vOpen(m_doc, &m_device, m_page_gap);
	if (m_layout)
	{
		m_layout->vClose();
		delete m_layout;
	}
	m_layout = layout;
	m_layout->vResize(size.Width, size.Height);
	if (pos.pageno >= 0)
	{
		if (view_mode == 3 || view_mode == 4 || view_mode == 6)
		{
			m_layout->vGotoPage(pos.pageno);
		}
		else
		{
			m_layout->vSetPos(0, 0, pos);
			m_layout->vMoveEnd();
		}
	}
}

void DXView::PDFClose()
{
	if (m_layout)
	{
		m_layout->vClose();
		delete m_layout;
	}
	m_layout = NULL;
	m_doc = nullptr;
	m_listener = nullptr;
	m_status = STA_NONE;
}

void DXView::PDFEndAnnot()
{
	if (m_status != STA_ANNOT) return;
	m_annot_page = NULL;
	m_annot_pos.pageno = -1;
	m_annot = nullptr;
	onVDrawCanvas();
	m_status = STA_NONE;
	//radaee: dismiss editbox
	//radaee: dismiss droplist
	if (m_listener)
		m_listener->OnPDFAnnotTapped(-1, nullptr);
}

void DXView::PDFPerformAnnot()
{
	if (m_status != STA_ANNOT) return;
	PDFPage ^page = m_doc->GetPage(m_annot_page->GetPageNo());
	if (!page || !m_annot) return;
	page->ObjsStart();
	int dest = m_annot->Dest;
	if (dest >= 0) {
		m_layout->vGotoPage(dest);
		onVDrawCanvas();
	}
	String ^js = m_annot->JS;
	if (m_listener && js && !js->IsEmpty()) m_listener->OnPDFOpenJS(js);
	String ^uri = m_annot->URI;
	if (m_listener && uri && !uri->IsEmpty())
		m_listener->OnPDFOpenURI(uri);
	int index;
	String ^mov = m_annot->GetMovieName();
	if (m_listener && mov && !mov->IsEmpty())
		m_listener->OnPDFOpenMovie(m_annot, mov);
	String ^snd = m_annot->GetSoundName();
	if (m_listener && snd && !snd->IsEmpty())
		m_listener->OnPDFOpenSound(m_annot, snd);
	String ^att = m_annot->GetAttachmentName();
	if (m_listener && att && !att->IsEmpty())
		m_listener->OnPDFOpenAttachment(m_annot, att);
	String ^f3d = m_annot->Get3DName();
	if (m_listener && f3d && !f3d->IsEmpty())
		m_listener->OnPDFOpen3D(m_annot, f3d);

	bool reset = m_annot->IsResetButton();
	if (reset && m_doc->CanSave)
	{
		m_annot->DoReset();
		m_layout->vRenderPage(m_annot_page);
		if (m_listener)
			m_listener->OnPDFPageModified(m_annot_page->GetPageNo());
	}
	String ^tar = m_annot->SubmitTarget;
	if (m_listener && tar && !tar->IsEmpty())
		m_listener->OnPDFOpenURI(tar + "?" + m_annot->SubmitPara);
	page->Close();
	PDFEndAnnot();
}

void DXView::PDFFindStart(String ^key, boolean match_case, boolean whole_word)
{
	m_layout->vFindStart(key, match_case, whole_word);
}

void DXView::PDFFind(int dir)
{
	m_layout->vFind(dir);
}

void DXView::PDFFindEnd()
{
	m_layout->vFindEnd();
}

bool DXView::PDFSetSelMarkup(int type)
{
	if (m_status == STA_SELECT && m_sel && m_sel->SetSelMarkup(type))
	{
		//add to redo/undo stack.
		PDFPage ^page = m_sel->GetPage();
		m_opstack.push(new OPAdd(m_annot_page->GetPageNo(), page, page->AnnotCount - 1));
		m_layout->vRenderPage(m_annot_page);
		onVDrawCanvas();
		if (m_listener)
			m_listener->OnPDFPageModified(m_annot_page->GetPageNo());
		return true;
	}
	else {
		return false;
	}
}
void DXView::PDFGotoPage(int pageno)
{
	if (!m_layout) return;
	m_layout->vGotoPage(pageno);
}

bool DXView::PDFUndo()
{
	//if(m_opstack.can_undo()) return;
	OPItem *item = m_opstack.undo();
	if (item)
	{
		item->op_undo(m_doc);
		PDFGotoPage(item->m_pageno);
		m_layout->vRenderPage(m_layout->vGetPage(item->m_pageno));
	}
	//radaee: show tip to know no more undo
	//else Toast.makeText(getContext(), "No more undo.", Toast.LENGTH_SHORT).show();
	return item != NULL;
}

bool DXView::PDFRedo()
{
	//if(m_opstack.can_redo()) return;
	OPItem *item = m_opstack.redo();
	if (item) {
		item->op_redo(m_doc);
		PDFGotoPage(item->m_pageno);
		m_layout->vRenderPage(m_layout->vGetPage(item->m_pageno));
	}
	//radaee: show tip to know no more redo
	//else Toast.makeText(getContext(), "No more redo.", Toast.LENGTH_SHORT).show();
	return item != NULL;
}

void DXView::PDFSetInk(DXOPCODE code)
{
	if (code == DXOPCODE::OP_START)//start
	{
		m_status = STA_INK;
		m_ink = ref new PDFInk(3, 0xFFFF0000);
	}
	else if (code == DXOPCODE::OP_END)//end
	{
		m_status = STA_NONE;
		if (m_annot_page)
		{
			PDFPage ^page = m_doc->GetPage(m_annot_page->GetPageNo());
			if (page)
			{
				page->ObjsStart();
				PDFMatrix ^mat = m_annot_page->CreateInvertMatrix(m_layout->vGetX(), m_layout->vGetY());
				mat->TransformInk(m_ink);
				page->AddAnnotInk(m_ink);
				//add to redo/undo stack.
				m_opstack.push(new OPAdd(m_annot_page->GetPageNo(), page, page->AnnotCount - 1));
				m_layout->vRenderPage(m_annot_page);
				page->Close();
				if (m_listener) m_listener->OnPDFPageModified(m_annot_page->GetPageNo());
			}
		}
		m_ink = nullptr;
		m_annot_page = NULL;
		onVDrawCanvas();
	}
	else//cancel
	{
		m_status = STA_NONE;
		m_ink = nullptr;
		m_annot_page = NULL;
		onVDrawCanvas();
	}
}

void DXView::PDFSetRect(DXOPCODE code)
{
	if (code == DXOPCODE::OP_START)//start
	{
		m_status = STA_RECT;
	}
	else if (code == DXOPCODE::OP_END)//end
	{
		if (m_rects)
		{
			int len = m_rects_cnt;
			int cur;
			DXPageSet pset(len);
			for (cur = 0; cur < len; cur += 4)
			{
				PDFPos pos;
				m_layout->vGetPos((int)m_rects[cur], (int)m_rects[cur + 1], pos);
				DXPage *vpage = m_layout->vGetPage(pos.pageno);
				PDFPage ^page = m_doc->GetPage(vpage->GetPageNo());
				if (page)
				{
					page->ObjsStart();
					PDFMatrix ^mat = vpage->CreateInvertMatrix(m_layout->vGetX(), m_layout->vGetY());
					PDFRect rect;
					if (m_rects[cur] > m_rects[cur + 2])
					{
						rect.left = m_rects[cur + 2];
						rect.right = m_rects[cur];
					}
					else
					{
						rect.left = m_rects[cur];
						rect.right = m_rects[cur + 2];
					}
					if (m_rects[cur + 1] > m_rects[cur + 3])
					{
						rect.top = m_rects[cur + 3];
						rect.bottom = m_rects[cur + 1];
					}
					else {
						rect.top = m_rects[cur + 1];
						rect.bottom = m_rects[cur + 3];
					}
					mat->TransformRect(rect);
					page->AddAnnotRect(rect, vpage->ToPDFSize(3), 0x80FF0000, 0x800000FF);
					//add to redo/undo stack.
					m_opstack.push(new OPAdd(pos.pageno, page, page->AnnotCount - 1));
					pset.Insert(vpage);
					page->Close();
				}
			}
			for (cur = 0; cur < pset.pages_cnt; cur++)
			{
				DXPage *vpage = pset.pages[cur];
				m_layout->vRenderPage(vpage);
				if (m_listener)
					m_listener->OnPDFPageModified(vpage->GetPageNo());
			}
		}
		m_status = STA_NONE;
		free(m_rects);
		m_rects = NULL;
		m_rects_cnt = 0;
		onVDrawCanvas();
	}
	else//cancel
	{
		m_status = STA_NONE;
		free(m_rects);
		m_rects = NULL;
		m_rects_cnt = 0;
		onVDrawCanvas();
	}
}

void DXView::PDFSetEllipse(DXOPCODE code)
{
	if (code == DXOPCODE::OP_START)//start
	{
		m_status = STA_ELLIPSE;
	}
	else if (code == DXOPCODE::OP_END)//end
	{
		if (m_rects)
		{
			int len = m_rects_cnt;
			int cur;
			DXPageSet pset(len);
			for (cur = 0; cur < len; cur += 4)
			{
				PDFPos pos;
				m_layout->vGetPos((int)m_rects[cur], (int)m_rects[cur + 1], pos);
				DXPage *vpage = m_layout->vGetPage(pos.pageno);
				PDFPage ^page = m_doc->GetPage(vpage->GetPageNo());
				if (page)
				{
					page->ObjsStart();
					PDFMatrix ^mat = vpage->CreateInvertMatrix(m_layout->vGetX(), m_layout->vGetY());
					PDFRect rect;
					if (m_rects[cur] > m_rects[cur + 2])
					{
						rect.left = m_rects[cur + 2];
						rect.right = m_rects[cur];
					}
					else
					{
						rect.left = m_rects[cur];
						rect.right = m_rects[cur + 2];
					}
					if (m_rects[cur + 1] > m_rects[cur + 3])
					{
						rect.top = m_rects[cur + 3];
						rect.bottom = m_rects[cur + 1];
					}
					else {
						rect.top = m_rects[cur + 1];
						rect.bottom = m_rects[cur + 3];
					}
					mat->TransformRect(rect);
					page->AddAnnotEllipse(rect, vpage->ToPDFSize(3), 0x80FF0000, 0x800000FF);
					//add to redo/undo stack.
					m_opstack.push(new OPAdd(pos.pageno, page, page->AnnotCount - 1));
					pset.Insert(vpage);
					page->Close();
				}
			}
			for (cur = 0; cur < pset.pages_cnt; cur++)
			{
				DXPage *vpage = pset.pages[cur];
				m_layout->vRenderPage(vpage);
				if (m_listener)
					m_listener->OnPDFPageModified(vpage->GetPageNo());
			}
		}
		m_status = STA_NONE;
		free(m_rects);
		m_rects = NULL;
		m_rects_cnt = 0;
		onVDrawCanvas();
	}
	else//cancel
	{
		m_status = STA_NONE;
		free(m_rects);
		m_rects = NULL;
		m_rects_cnt = 0;
		onVDrawCanvas();
	}
}
void DXView::PDFSetSelect(DXOPCODE code)
{
	if (code == DXOPCODE::OP_START)
	{
		m_annot_page = NULL;
		m_status = STA_SELECT;
		onVDrawCanvas();
	}
	else
	{
		m_annot_page = NULL;
		if (m_sel) delete m_sel;
		m_sel = NULL;
		m_status = STA_NONE;
		onVDrawCanvas();
	}
}

void DXView::PDFSetNote(DXOPCODE code)
{
	if (code == DXOPCODE::OP_START)
	{
		m_notes = NULL;
		m_notes_cnt = 0;
		m_status = STA_NOTE;
	}
	else if (code == DXOPCODE::OP_END)//end
	{
		if (m_listener && m_notes)
		{
			int cur = 0;
			int cnt = m_notes_cnt;
			while (cur < cnt)
			{
				m_listener->OnPDFPageModified(m_notes[cur].page->GetPageNo());
				cur++;
			}
		}
		free(m_notes);
		m_notes = NULL;
		m_notes_cnt = 0;
		m_status = STA_NONE;
	}
	else//cancel
	{
		if (m_notes)//remove added note.
		{
			int cur = 0;
			int cnt = m_notes_cnt;
			while (cur < cnt) {
				DXPage *vpage = m_notes[cur].page;
				PDFPage ^page = m_doc->GetPage(vpage->GetPageNo());
				page->ObjsStart();
				int index = m_notes[cur].index;
				PDFAnnot ^annot;
				while (annot = page->GetAnnot(index))
				{
					annot->RemoveFromPage();
					m_opstack.undo();
				}
				page->Close();
				m_layout->vRenderPage(vpage);
				cur++;
			}
		}
		free(m_notes);
		m_notes = NULL;
		m_notes_cnt = 0;
		m_status = STA_NONE;
	}
}

void DXView::PDFSetLine(DXOPCODE code)
{
	if (code == DXOPCODE::OP_START)//start
	{
		m_status = STA_LINE;
	}
	else if (code == DXOPCODE::OP_END)//end
	{
		if (m_rects)
		{
			int len = m_rects_cnt;
			int cur;
			DXPageSet pset(len);
			for (cur = 0; cur < len; cur += 4)
			{
				PDFPos pos;
				m_layout->vGetPos((int)m_rects[cur], (int)m_rects[cur + 1], pos);
				DXPage *vpage = m_layout->vGetPage(pos.pageno);
				PDFPoint pt1;
				PDFPoint pt2;
				pt1.x = m_rects[cur];
				pt1.y = m_rects[cur + 1];
				pt2.x = m_rects[cur + 2];
				pt2.y = m_rects[cur + 3];
				PDFPage ^page = m_doc->GetPage(vpage->GetPageNo());
				if (page)
				{
					page->ObjsStart();
					PDFMatrix ^mat = vpage->CreateInvertMatrix(m_layout->vGetX(), m_layout->vGetY());
					pt1 = mat->TransformPoint(pt1);
					pt2 = mat->TransformPoint(pt2);
					page->AddAnnotLine(pt1.x, pt1.y, pt2.x, pt2.y, 1, 0, vpage->ToPDFSize(3), 0x80FF0000, 0x800000FF);
					//add to redo/undo stack.
					m_opstack.push(new OPAdd(pos.pageno, page, page->AnnotCount - 1));
					page->Close();
					pset.Insert(vpage);
				}
			}
			for (cur = 0; cur < pset.pages_cnt; cur++) {
				DXPage *vpage = pset.pages[cur];
				m_layout->vRenderPage(vpage);
				if (m_listener)
					m_listener->OnPDFPageModified(vpage->GetPageNo());
			}
		}
		m_status = STA_NONE;
		free(m_rects);
		m_rects = NULL;
		m_rects_cnt = 0;
		onVDrawCanvas();
	}
	else//cancel
	{
		m_status = STA_NONE;
		free(m_rects);
		m_rects = NULL;
		m_rects_cnt = 0;
		onVDrawCanvas();
	}
}

void DXView::PDFSetStamp(DXOPCODE code)
{
	if (code == DXOPCODE::OP_START)//start
	{
		m_status = STA_STAMP;
	}
	else if (code == DXOPCODE::OP_END)//end
	{
		m_status = STA_NONE;
		free(m_rects);
		m_rects = NULL;
		m_rects_cnt = 0;
		onVDrawCanvas();
	}
	else//cancel
	{
		m_status = STA_NONE;
		free(m_rects);
		m_rects = NULL;
		m_rects_cnt = 0;
		onVDrawCanvas();
	}
}
