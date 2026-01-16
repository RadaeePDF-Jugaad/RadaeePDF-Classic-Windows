using System;
using Windows.Foundation;
using Windows.UI;
using Windows.UI.Input;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;

namespace com.radaee.reader
{
    class PDFPopup : RelativePanel
    {
        private RelativePanel m_parent;
        private FrameworkElement m_content;
        public PDFPopup(RelativePanel parent, FrameworkElement content, Rect rect) : base()
        {
            m_parent = parent;
            m_content = content;
            Children.Add(m_content);
            m_parent.Children.Add(this);
            PointerPressed += vOnTouchDown;
            PointerReleased += vOnTouchUp;
            PointerCanceled += vOnTouchUp;
            SetValue(RelativePanel.AlignLeftWithPanelProperty, true);
            SetValue(RelativePanel.AlignTopWithPanelProperty, true);
            SetValue(RelativePanel.AlignRightWithPanelProperty, true);
            SetValue(RelativePanel.AlignBottomWithPanelProperty, true);
            Color clr;
            clr.A = 0;
            clr.R = 0;
            clr.G = 0;
            clr.B = 0;
            Background = new SolidColorBrush(clr);
            BorderBrush = new SolidColorBrush(Color.FromArgb(255, 64, 64, 64));
            Thickness tik;
            tik.Left = 1;
            tik.Right = 1;
            tik.Top = 1;
            tik.Bottom = 1;
            BorderThickness = tik;
            m_content.SetValue(RelativePanel.WidthProperty, rect.Width);
            m_content.SetValue(RelativePanel.HeightProperty, rect.Height);
            TranslateTransform trans = new TranslateTransform();
            trans.X = rect.X;
            trans.Y = rect.Y;
            m_content.RenderTransform = trans;
        }
        private void vOnTouchDown(Object sender, PointerRoutedEventArgs e)
        {
            PointerPoint point = e.GetCurrentPoint(m_content);
            Point pt = point.Position;
            double w = m_content.ActualWidth;
            double h = m_content.ActualHeight;
            if (pt.X < 0 || pt.Y < 0 || pt.X > w || pt.Y > h)
                dismiss();
        }
        private void vOnTouchUp(Object sender, PointerRoutedEventArgs e)
        {
            PointerPoint point = e.GetCurrentPoint(m_content);
            Point pt = point.Position;
            double w = m_content.ActualWidth;
            double h = m_content.ActualHeight;
            if (pt.X < 0 || pt.Y < 0 || pt.X > w || pt.Y > h)
                dismiss();
        }
        public void dismiss()
        {
            Children.Remove(m_content);
            m_parent.Children.Remove(this);
            PointerPressed -= vOnTouchDown;
            PointerReleased -= vOnTouchUp;
            PointerCanceled -= vOnTouchUp;
        }
    }
}
