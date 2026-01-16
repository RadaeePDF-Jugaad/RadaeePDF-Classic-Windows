using com.radaee.reader;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;

namespace RDPDFReader.annotui
{
    class UILHeadView : Button
    {
        public UILHeadView() : base()
        {
            m_bmp = new WriteableBitmap(60, 30);
            m_img = new Image();
            m_img.Width = 60;
            m_img.Height = 30;
            Content = m_img;
            Padding = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
            BorderThickness = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
            setHead(0);
        }
        private WriteableBitmap m_bmp;
        private int m_head;
        private Image m_img;
        public int getHead()
        {
            return m_head;
        }
        public void setHead(int head)
        {
            m_head = head;
            RDPDFLib.pdf.PDFGlobal.drawLineHead(m_head, m_bmp);
            m_img.Source = m_bmp;
            m_bmp.Invalidate();
        }
    }
    class UILHeadButton : Button
    {
        public UILHeadButton() : base()
        {
            m_bmp = new WriteableBitmap(60, 30);
            m_img = new Image();
            m_img.Width = 60;
            m_img.Height = 30;
            Content = m_img;
            Padding = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
            BorderThickness = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
            setHead(0);

            m_panel = new StackPanel();
            m_panel.Width = 60;
            m_panel.Height = 10 * 30;
            m_panel.Orientation = Orientation.Vertical;

            m_scroller = new ScrollViewer();
            m_scroller.Width = 60;
            m_scroller.Height = 90;
            m_scroller.Content = m_panel;

            RoutedEventHandler click = (object sender, RoutedEventArgs e) =>
            {
                UILHeadView vv = (UILHeadView)sender;
                setHead(vv.getHead());
                if (m_popup != null)
                {
                    m_popup.dismiss();
                    m_popup = null;
                }
            };

            for (int ih = 0; ih < 10; ih++)
            {
                UILHeadView vw = new UILHeadView();
                vw.Width = 60;
                vw.Height = 30;
                vw.Click += click;
                vw.setHead(ih);
                m_panel.Children.Add(vw);
            }
        }
        private WriteableBitmap m_bmp;
        private int m_head;
        private Image m_img;
        private ScrollViewer m_scroller;
        private StackPanel m_panel;
        public int getHead()
        {
            return m_head;
        }
        public void setHead(int head)
        {
            m_head = head;
            RDPDFLib.pdf.PDFGlobal.drawLineHead(m_head, m_bmp);
            m_img.Source = m_bmp;
            m_bmp.Invalidate();
        }
        private RelativePanel m_root;
        private PDFPopup m_popup;
        public void init(RelativePanel root)
        {
            m_root = root;
            Click += (object sender, RoutedEventArgs e) =>
            {
                if (m_popup != null) m_popup.dismiss();
                GeneralTransform trans = TransformToVisual(m_root);
                Point pt;
                //pt.X = ActualWidth;
                pt.X = 0;
                pt.Y = 0;
                pt = trans.TransformPoint(pt);
                Rect rect = new Rect(pt.X, pt.Y, 60, 90);
                m_popup = new PDFPopup(m_root, m_scroller, rect);
            };
        }
    }
}
