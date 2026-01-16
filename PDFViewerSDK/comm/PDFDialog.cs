using Windows.Foundation;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace com.radaee.reader
{
    class PDFDialog : ContentDialog
    {
        //private RelativePanel m_parent;
        //private FrameworkElement m_content;
        //private Button m_btn_ok;
        //private Button m_btn_cancel;
        public PDFDialog(/*RelativePanel parent,*/ FrameworkElement content, /*Rect rect,*/ TypedEventHandler<ContentDialog, ContentDialogButtonClickEventArgs> onOk, /*RoutedEventHandler*/TypedEventHandler<ContentDialog, ContentDialogButtonClickEventArgs> onCancel) : base()
        {
            //m_parent = parent;
            //m_content = content;
            Content = content;
            PrimaryButtonText = "OK";
            PrimaryButtonClick += onOk;
            SecondaryButtonText = "Cancel";
            SecondaryButtonClick += onCancel;
            Background = new SolidColorBrush(Color.FromArgb(255, 224, 224, 224));
            //Children.Add(m_content);
            //m_parent.Children.Add(this);
            //SetValue(RelativePanel.AlignLeftWithPanelProperty, true);
            //SetValue(RelativePanel.AlignTopWithPanelProperty, true);
            //SetValue(RelativePanel.AlignRightWithPanelProperty, true);
            //SetValue(RelativePanel.AlignBottomWithPanelProperty, true);
            //Color clr;
            //TranslateTransform trans;
            //clr.A = 64;
            //clr.R = 0;
            //clr.G = 0;
            //clr.B = 0;
            //Background = new SolidColorBrush(clr);
            //m_content.SetValue(RelativePanel.WidthProperty, rect.Width);
            //m_content.SetValue(RelativePanel.HeightProperty, rect.Height);
            //trans = new TranslateTransform();
            //trans.X = rect.X;
            //trans.Y = rect.Y;
            //m_content.RenderTransform = trans;

            //m_btn_ok = new Button();
            //m_btn_ok.SetValue(RelativePanel.WidthProperty, rect.Width * 0.5);
            //m_btn_ok.SetValue(RelativePanel.HeightProperty, 40);

            ////m_btn_ok.Style = Application.Current.Resources["ButtonBase"] as Style;
            //m_btn_ok.Content = "OK";
            //m_btn_ok.Click += onOk;
            //trans = new TranslateTransform();
            //trans.X = rect.X;
            //trans.Y = rect.Bottom;
            //m_btn_ok.RenderTransform = trans;
            //m_btn_ok.Background = new SolidColorBrush(Color.FromArgb(255, 224, 224, 224));
            //Children.Add(m_btn_ok);

            //m_btn_cancel = new Button();
            //m_btn_cancel.SetValue(RelativePanel.WidthProperty, rect.Width * 0.5);
            //m_btn_cancel.SetValue(RelativePanel.HeightProperty, 40);

            ////m_btn_cancel.Style = Application.Current.Resources["ButtonBase"] as Style;
            //m_btn_cancel.Content = "Cancel";
            //m_btn_cancel.Click += onCancel;
            //trans = new TranslateTransform();
            //trans.X = rect.X + rect.Width / 2;
            //trans.Y = rect.Bottom;
            //m_btn_cancel.RenderTransform = trans;
            //m_btn_cancel.Background = new SolidColorBrush(Color.FromArgb(255, 224, 224, 224));
            //Children.Add(m_btn_cancel);
        }


        //public void dismiss()
        //{
        //    m_parent.Children.Remove(this);
        //}
    }
    class PDFDialogSave : ContentDialog
    {
        //private RelativePanel m_parent;
        //private FrameworkElement m_content;
        //private Button m_btn_ok;
        //private Button m_btn_cancel;
        public PDFDialogSave(/*RelativePanel parent,*/ UserControl content, /*Rect rect,*/ TypedEventHandler<ContentDialog, ContentDialogButtonClickEventArgs> onOk, TypedEventHandler<ContentDialog, ContentDialogButtonClickEventArgs> onNo, /*RoutedEventHandler*/TypedEventHandler<ContentDialog, ContentDialogButtonClickEventArgs> onCancel) : base()
        {
            //m_parent = parent;
            //m_content = content;
            Content = content;
            PrimaryButtonText = "YES";
            PrimaryButtonClick += onOk;
            SecondaryButtonText = "NO";
            SecondaryButtonClick += onNo;
            CloseButtonText = "Cancel";
            CloseButtonClick += onCancel;
            Background = new SolidColorBrush(Color.FromArgb(255, 224, 224, 224));
            //Children.Add(m_content);
            //m_parent.Children.Add(this);
            //SetValue(RelativePanel.AlignLeftWithPanelProperty, true);
            //SetValue(RelativePanel.AlignTopWithPanelProperty, true);
            //SetValue(RelativePanel.AlignRightWithPanelProperty, true);
            //SetValue(RelativePanel.AlignBottomWithPanelProperty, true);
            //Color clr;
            //TranslateTransform trans;
            //clr.A = 64;
            //clr.R = 0;
            //clr.G = 0;
            //clr.B = 0;
            //Background = new SolidColorBrush(clr);
            //m_content.SetValue(RelativePanel.WidthProperty, rect.Width);
            //m_content.SetValue(RelativePanel.HeightProperty, rect.Height);
            //trans = new TranslateTransform();
            //trans.X = rect.X;
            //trans.Y = rect.Y;
            //m_content.RenderTransform = trans;

            //m_btn_ok = new Button();
            //m_btn_ok.SetValue(RelativePanel.WidthProperty, rect.Width * 0.5);
            //m_btn_ok.SetValue(RelativePanel.HeightProperty, 40);

            ////m_btn_ok.Style = Application.Current.Resources["ButtonBase"] as Style;
            //m_btn_ok.Content = "OK";
            //m_btn_ok.Click += onOk;
            //trans = new TranslateTransform();
            //trans.X = rect.X;
            //trans.Y = rect.Bottom;
            //m_btn_ok.RenderTransform = trans;
            //m_btn_ok.Background = new SolidColorBrush(Color.FromArgb(255, 224, 224, 224));
            //Children.Add(m_btn_ok);

            //m_btn_cancel = new Button();
            //m_btn_cancel.SetValue(RelativePanel.WidthProperty, rect.Width * 0.5);
            //m_btn_cancel.SetValue(RelativePanel.HeightProperty, 40);

            ////m_btn_cancel.Style = Application.Current.Resources["ButtonBase"] as Style;
            //m_btn_cancel.Content = "Cancel";
            //m_btn_cancel.Click += onCancel;
            //trans = new TranslateTransform();
            //trans.X = rect.X + rect.Width / 2;
            //trans.Y = rect.Bottom;
            //m_btn_cancel.RenderTransform = trans;
            //m_btn_cancel.Background = new SolidColorBrush(Color.FromArgb(255, 224, 224, 224));
            //Children.Add(m_btn_cancel);
        }


        //public void dismiss()
        //{
        //    m_parent.Children.Remove(this);
        //}
    }
}
