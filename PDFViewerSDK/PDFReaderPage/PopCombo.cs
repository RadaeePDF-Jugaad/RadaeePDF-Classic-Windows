using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;

namespace RDPDFReader.annotui
{
    class PopCombo
    {
        private RelativePanel m_parent;
        private ScrollViewer m_scroller;
        private StackPanel m_panel;
        public PopCombo()
        {
            m_panel = new StackPanel();
            m_panel.Orientation = Orientation.Vertical;
            m_panel.Background = new SolidColorBrush(Color.FromArgb(255, 255, 255, 160));
            m_scroller = new ScrollViewer();
            m_scroller.Content = m_panel;
        }
        private void reset()
        {
            while(m_panel.Children.Count > 0)
                m_panel.Children.RemoveAt(0);
        }
        public void show(RelativePanel parent, Rect rect, float scale, String[] opts, RoutedEventHandler tapped)
        {
            reset();
            m_parent = parent;
            parent.Children.Add(m_scroller);

            int cnt = opts.Length;

            TranslateTransform trans = new TranslateTransform();
            trans.X = rect.Left;
            trans.Y = rect.Bottom;
            m_scroller.RenderTransform = trans;
            m_scroller.Width = rect.Width;
            m_panel.Width = rect.Width;
            float item_height = scale * 16;
            float total_height = item_height * cnt;
            m_panel.Height = total_height;
            if (total_height > item_height * 5) total_height = item_height * 5;
            m_scroller.Height = total_height;

            for(int cur = 0; cur < cnt; cur++)
            {
                TextBlock txt = new TextBlock();
                txt.Width = rect.Width;
                txt.Height = item_height;
                txt.FontSize = scale * 12;
                txt.Text = opts[cur];

                Button btn = new Button();
                btn.Tag = cur;
                btn.Content = txt;
                btn.Click += tapped;
                btn.Padding = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
                btn.BorderThickness = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
                btn.Height = item_height;
                m_panel.Children.Add(btn);
            }

            m_scroller.UpdateLayout();
            m_scroller.InvalidateMeasure();
        }
        public void dismiss()
        {
            if (m_parent != null)
                m_parent.Children.Remove(m_scroller);
        }
    }
}
