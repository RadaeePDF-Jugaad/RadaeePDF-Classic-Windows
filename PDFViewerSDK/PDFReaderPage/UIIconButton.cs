using com.radaee.reader;
using RDPDFLib.pdf;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;

namespace RDPDFReader.annotui
{
    class UIIconView : Button
    {
        private static String[] ms_text_icon_names =
        {
	        "Note",
	        "Comment",
	        "Key",
	        "Help",
	        "NewParagraph",
	        "Paragraph",
	        "Insert",
	        "Check",
	        "Circle",
	        "Cross",
	        "CrossHairs",
	        "RightArrow",
	        "RightPointer",
	        "Star",
	        "UpArrow",
	        "UpLeftArrow",
        };
        private static String[] ms_attach_icon_names =
        {
            "PushPin",
	        "Graph",
	        "Paperclip",
	        "Tag"
        };

        public UIIconView() : base()
        {
            m_bmp = new WriteableBitmap(48, 48);
            m_img = new Image();
            m_label = new TextBlock();
            m_img.Width = 30;
            m_img.Height = 30;
            m_label.Width = 120;

            m_panel = new StackPanel();
            m_panel.Orientation = Orientation.Horizontal;
            m_panel.Children.Add(m_img);
            m_panel.Children.Add(m_label);
            m_panel.Background = new SolidColorBrush(Color.FromArgb(255, 255, 255, 255));

            Content = m_panel;
            Padding = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
            BorderThickness = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
        }
        private WriteableBitmap m_bmp;
        private int m_atype;
        private int m_icon;
        private StackPanel m_panel;
        private Image m_img;
        private TextBlock m_label;
        public void init(int atype, int icon)
        {
            m_atype = atype;
            setIcon(icon);
        }
        public int getIcon()
        {
            return m_icon;
        }
        public void setIcon(int icon)
        {
            m_icon = icon;
            RDPDFLib.pdf.PDFGlobal.drawAnnotIcon(m_atype, m_icon, m_bmp);
            m_img.Source = m_bmp;
            m_bmp.Invalidate();
            if (m_atype == 1)
                m_label.Text = ms_text_icon_names[m_icon];
            else if(m_atype == 17)
                m_label.Text = ms_attach_icon_names[m_icon];
        }
    }
    class UIIconButton : Button
    {
        public UIIconButton() : base()
        {
            m_bmp = new WriteableBitmap(48, 48);
            m_img = new Image();
            m_img.Width = 30;
            m_img.Height = 30;
            Content = m_img;
            Padding = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
            BorderThickness = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
        }
        private WriteableBitmap m_bmp;
        private int m_atype;
        private int m_icon;
        private Image m_img;
        private ScrollViewer m_scroller;
        private StackPanel m_panel;
        private RelativePanel m_root;
        private PDFPopup m_popup;
        public void init(RelativePanel root, int atype, int icon)
        {
            m_root = root;
            m_atype = atype;
            setIcon(icon);

            m_panel = new StackPanel();
            m_panel.Width = 150;
            m_panel.Height = 16 * 30;
            m_panel.Orientation = Orientation.Vertical;

            RoutedEventHandler click = (object sender, RoutedEventArgs e) =>
            {
                UIIconView vv = (UIIconView)sender;
                setIcon(vv.getIcon());
                if (m_popup != null)
                {
                    m_popup.dismiss();
                    m_popup = null;
                }
            };

            for (int ih = 0; ih < 16; ih++)
            {
                UIIconView vw = new UIIconView();
                vw.Width = 150;
                vw.Height = 30;
                vw.Click += click;
                vw.init(m_atype, ih);
                m_panel.Children.Add(vw);
            }
            setIcon(0);

            m_scroller = new ScrollViewer();
            m_scroller.Width = 150;
            m_scroller.Height = 210;
            m_scroller.Content = m_panel;

            Click += (object sender, RoutedEventArgs e) =>
            {
                if (m_popup != null) m_popup.dismiss();
                GeneralTransform trans = TransformToVisual(m_root);
                Point pt;
                //pt.X = ActualWidth;
                pt.X = 0;
                pt.Y = 0;
                pt = trans.TransformPoint(pt);
                Rect rect = new Rect(pt.X, pt.Y, 150, 210);
                m_popup = new PDFPopup(m_root, m_scroller, rect);
            };
        }
        public int getIcon()
        {
            return m_icon;
        }
        public void setIcon(int icon)
        {
            m_icon = icon;
            RDPDFLib.pdf.PDFGlobal.drawAnnotIcon(m_atype, m_icon, m_bmp);
            m_img.Source = m_bmp;
            m_bmp.Invalidate();
        }
    }
}
