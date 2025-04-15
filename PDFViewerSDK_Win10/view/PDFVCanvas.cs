using RDPDFLib.pdf;
using Windows.Foundation;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Shapes;
using static PDFViewerSDK_Win10.view.PDFView;

namespace PDFViewerSDK_Win10
{
    namespace view
    {
        public class PDFVLayout : Canvas
        {
            public PDFVLayout(ScrollViewer parent)
                : base()
            {
                IsHitTestVisible = false;
                parent.Content = this;
                Background = null;
            }
            public void resize(float w, float h)
            {
                Width = w;
                Height = h;
                UpdateLayout();
            }
        }
        public class PDFVCanvas : Canvas
        {
            private float m_scale = 1;
            private Canvas m_parent;
            public bool m_isVerticalMode;
            private PDFView m_handle;
            public PDFVCanvas(Canvas parent, PDFView handle)
                : base()
            {
                IsHitTestVisible = false;
                parent.Children.Add(this);
                m_handle = handle;
                m_parent = parent;
                Visibility = Visibility.Visible;
                m_isVerticalMode = false;
            }
            public void set_scale(float scale)
            {
                m_scale = scale;
            }
            public void fill_rect(PDFRect rect, Color clr)
            {
                Rectangle rect1 = new Rectangle();
                rect1.SetValue(Canvas.LeftProperty, rect.left * m_scale);
                rect1.SetValue(Canvas.TopProperty, rect.top * m_scale);
                rect1.Width = (rect.right - rect.left) * m_scale;
                rect1.Height = (rect.bottom - rect.top) * m_scale;
                rect1.Fill = new SolidColorBrush(clr);
                Children.Add(rect1);
            }
            public void draw_rect(PDFRect rect, float width, uint color)
            {
                Rectangle rect1 = new Rectangle();
                Color clr;
                clr.A = (byte)(color >> 24);
                clr.R = (byte)(color >> 16);
                clr.G = (byte)(color >> 8);
                clr.B = (byte)(color);
                rect1.Stroke = new SolidColorBrush(clr);
                rect1.StrokeThickness = width;
                rect1.SetValue(Canvas.LeftProperty, rect.left);
                rect1.SetValue(Canvas.TopProperty, rect.top);
                rect1.Width = rect.right - rect.left;
                rect1.Height = rect.bottom - rect.top;
                Children.Add(rect1);
            }
            public void draw_rects(PDFPoint[] pts, int cnt, float width, uint color)
            {
                Color clr;
                clr.A = (byte)(color >> 24);
                clr.R = (byte)(color >> 16);
                clr.G = (byte)(color >> 8);
                clr.B = (byte)(color);
                SolidColorBrush br = new SolidColorBrush(clr);
                int cur = 0;
                cnt <<= 1;
                for (cur = 0; cur < cnt; cur += 2)
                {
                    PDFPoint pt0 = pts[cur];
                    PDFPoint pt1 = pts[cur + 1];
                    Rectangle rect = new Rectangle();
                    rect.StrokeThickness = width;
                    rect.Stroke = br;
                    if (pt0.x > pt1.x)
                    {
                        rect.SetValue(Canvas.LeftProperty, pt1.x);
                        rect.Width = pt0.x - pt1.x;
                    }
                    else
                    {
                        rect.SetValue(Canvas.LeftProperty, pt0.x);
                        rect.Width = pt1.x - pt0.x;
                    }
                    if (pt0.y > pt1.y)
                    {
                        rect.SetValue(Canvas.TopProperty, pt1.y);
                        rect.Height = pt0.y - pt1.y;
                    }
                    else
                    {
                        rect.SetValue(Canvas.TopProperty, pt0.y);
                        rect.Height = pt1.y - pt0.y;
                    }
                    Children.Add(rect);
                }
            }
            public void draw_ovals(PDFPoint[] pts, int cnt, float width, uint color)
            {
                Color clr;
                clr.A = (byte)(color >> 24);
                clr.R = (byte)(color >> 16);
                clr.G = (byte)(color >> 8);
                clr.B = (byte)(color);
                SolidColorBrush br = new SolidColorBrush(clr);
                int cur = 0;
                cnt <<= 1;
                float w = width * 0.5f;
                for (cur = 0; cur < cnt; cur += 2)
                {
                    PDFPoint pt0 = pts[cur];
                    PDFPoint pt1 = pts[cur + 1];
                    Ellipse rect = new Ellipse();
                    rect.StrokeThickness = width;
                    rect.Stroke = br;
                    if (pt0.x > pt1.x)
                    {
                        rect.SetValue(Canvas.LeftProperty, pt1.x - w);
                        rect.Width = pt0.x - pt1.x + width;
                    }
                    else
                    {
                        rect.SetValue(Canvas.LeftProperty, pt0.x - w);
                        rect.Width = pt1.x - pt0.x + width;
                    }
                    if (pt0.y > pt1.y)
                    {
                        rect.SetValue(Canvas.TopProperty, pt1.y - w);
                        rect.Height = pt0.y - pt1.y + width;
                    }
                    else
                    {
                        rect.SetValue(Canvas.TopProperty, pt0.y - w);
                        rect.Height = pt1.y - pt0.y + width;
                    }
                    Children.Add(rect);
                }
            }
            public void draw_lines(PDFPoint[] pts, int cnt, float width, uint color)
            {
                Color clr;
                clr.A = (byte)(color >> 24);
                clr.R = (byte)(color >> 16);
                clr.G = (byte)(color >> 8);
                clr.B = (byte)(color);
                SolidColorBrush br = new SolidColorBrush(clr);
                int cur = 0;
                cnt <<= 1;
                float w = width * 0.5f;
                for (cur = 0; cur < cnt; cur += 2)
                {
                    PDFPoint pt0 = pts[cur];
                    PDFPoint pt1 = pts[cur + 1];
                    Line line = new Line();
                    line.StrokeThickness = width;
                    line.Stroke = br;
                    line.SetValue(Line.X1Property, pt0.x);
                    line.SetValue(Line.X2Property, pt1.x);
                    line.SetValue(Line.Y1Property, pt0.y);
                    line.SetValue(Line.Y2Property, pt1.y);
                    Children.Add(line);
                }
            }
            public void clear()
            {
                Children.Clear();
            }
            public int draw_ink(PDFInk ink, Path path, int pos, float offsetx, float offsety)
            {
                int cur = 0;
                int cnt = ink.NodesCnt;
                PDFPoint pt;
                PathGeometry inkg = (PathGeometry)path.Data;
                PathFigure inkf = inkg.Figures[inkg.Figures.Count - 1];
                LineSegment line;
                BezierSegment bezier;
                Point ppt;
                for (cur = pos; cur < cnt; cur++)
                {
                    switch (ink.GetOP(cur))
                    {
                        case 0:
                            pt = ink.GetPoint(cur);
                            ppt.X = pt.x + offsetx;
                            ppt.Y = pt.y + offsety;
                            inkf.StartPoint = ppt;
                            break;
                        case 1:
                            pt = ink.GetPoint(cur);
                            line = new LineSegment();
                            ppt.X = pt.x + offsetx;
                            ppt.Y = pt.y + offsety;
                            line.Point = ppt;
                            inkf.Segments.Add(line);
                            break;
                        case 2:
                            pt = ink.GetPoint(cur);
                            bezier = new Windows.UI.Xaml.Media.BezierSegment();
                            ppt.X = pt.x + offsetx;
                            ppt.Y = pt.y + offsety;
                            bezier.Point1 = ppt;
                            bezier.Point2 = ppt;
                            pt = ink.GetPoint(cur + 1);
                            ppt.X = pt.x + offsetx;
                            ppt.Y = pt.y + offsety;
                            bezier.Point3 = ppt;
                            cur++;
                            inkf.Segments.Add(bezier);
                            break;
                    }
                }
                Children.Add(path);
                return cnt;
            }
            public void resize(float w, float h)
            {
                Width = w;
                Height = h;
                UpdateLayout();
            }
        }
    }
}