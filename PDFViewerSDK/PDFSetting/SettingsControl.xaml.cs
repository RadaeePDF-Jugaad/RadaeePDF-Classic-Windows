using com.radaee.reader;
using System;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace RDPDFSetting.ui
{
    public delegate void OnViewModeSelectedHandler(int mode);

    public sealed partial class SettingsControl : UserControl
    {
        public event OnViewModeSelectedHandler OnViewModeSelected;
        public SettingsControl()
        {
            this.InitializeComponent();

            refreshData();
        }

        private void refreshData()
        {
            mRenderQualityLabel.Text = "Normal";
            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Render_quality"))
            {
                int value = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Render_quality"]);
                if (value == 0)
                    mRenderQualityLabel.Text = "Poor";
                else if (value == 1)
                    mRenderQualityLabel.Text = "Normal";
                else if (value == 2)
                    mRenderQualityLabel.Text = "Best";
            }

            mViewModeLabel.Text = "Vertical Mode";
            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("View_mode"))
            {
                int viewMode = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["View_mode"]);

                if (viewMode == 0)
                    mViewModeLabel.Text = "Vertical Mode";
                if (viewMode == 1)
                    mViewModeLabel.Text = "Horizontal Mode";
                if (viewMode == 2)
                    mViewModeLabel.Text = "Dual Page Mode";
                if (viewMode == 3)
                    mViewModeLabel.Text = "Single Page Mode";
            }

            int inkWidth = Convert.ToInt32(PDFReader.inkWidth);
            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Ink_width"))
                inkWidth = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Ink_width"]);
            mInkWidthLabel.Text = inkWidth.ToString();

            uint inkColor = Convert.ToUInt32(PDFReader.inkColor); ;
            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("ink_color"))
                inkColor = Convert.ToUInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["ink_color"]);
            inkColor = inkColor & 0x00ffffff;
            byte red = Convert.ToByte(inkColor >> 16);
            byte green = Convert.ToByte((inkColor & 0xff00) >> 8);
            byte blue = Convert.ToByte(inkColor & 0xff);
            Color color = Color.FromArgb(0xff, red, green, blue);
            SolidColorBrush brush = new SolidColorBrush();
            brush.Color = color;
            mInkColorLabel.Fill = brush;

            int rectWidth = Convert.ToInt32(PDFReader.rectWidth);
            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Rect_width"))
                rectWidth = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Rect_width"]);
            mRectWidthLabel.Text = rectWidth.ToString();

            uint rectColor = Convert.ToUInt32(PDFReader.rectColor);
            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Rect_color"))
                rectColor = Convert.ToUInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Rect_color"]);
            rectColor = rectColor & 0x00ffffff;
            red = Convert.ToByte(rectColor >> 16);
            green = Convert.ToByte((rectColor & 0xff00) >> 8);
            blue = Convert.ToByte(rectColor & 0xff);
            color = Color.FromArgb(0xff, red, green, blue);
            brush = new SolidColorBrush();
            brush.Color = color;
            mRectColorLabel.Fill = brush;

            int ellipseWidth = Convert.ToInt32(PDFReader.ovalWidth);
            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Ellipse_width"))
                ellipseWidth = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Ellipse_width"]);
            mEllipseWidthLabel.Text = ellipseWidth.ToString();

            uint ellipseColor = Convert.ToUInt32(PDFReader.ovalColor);
            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Ellipse_color"))
                ellipseColor = Convert.ToUInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Ellipse_color"]);
            ellipseColor = ellipseColor & 0x00ffffff;
            red = Convert.ToByte(ellipseColor >> 16);
            green = Convert.ToByte((ellipseColor & 0xff00) >> 8);
            blue = Convert.ToByte(ellipseColor & 0xff);
            color = Color.FromArgb(0xff, red, green, blue);
            brush = new SolidColorBrush();
            brush.Color = color;
            mEllipseColorLabel.Fill = brush;
        }

        public void show()
        {
            if (!RadioGroupPopup.IsOpen)
            {
                RadioGroupPopup.IsOpen = true;
            }
        }

        public void dismiss()
        {
            if (RadioGroupPopup.IsOpen)
            {
                //OnDialogDismissed();
                RadioGroupPopup.IsOpen = false;
            }
        }

        private void mRenderQualityBtn_Tapped(object sender, TappedRoutedEventArgs e)
        {
        }



        private void mViewModeBtn_Tapped(object sender, TappedRoutedEventArgs e)
        {
        }

        private void mInkWidthBtn_Tapped(object sender, TappedRoutedEventArgs e)
        {
            LineWidthControl inkWidthControl = new LineWidthControl(LineWidthControl.LineType.TypeInk);
            inkWidthControl.OnLineWidthDialogClosed += refreshData;
            inkWidthControl.show();
        }

        private void mInkColorBtn_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ColorPickerControl inkColorPicker = new ColorPickerControl(ColorPickerControl.ColorType.TypeInk);
            inkColorPicker.OnColorPickerClosed += refreshData;
            inkColorPicker.show();
        }

        private void mRectWidthBtn_Tapped(object sender, TappedRoutedEventArgs e)
        {
            LineWidthControl rectWidthControl = new LineWidthControl(LineWidthControl.LineType.TypeRect);
            rectWidthControl.OnLineWidthDialogClosed += refreshData;
            rectWidthControl.show();
        }

        private void mRectColorBtn_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ColorPickerControl rectColorPicker = new ColorPickerControl(ColorPickerControl.ColorType.TypeRect);
            rectColorPicker.OnColorPickerClosed += refreshData;
            rectColorPicker.show();
        }

        private void mEllipseWidthBtn_Tapped(object sender, TappedRoutedEventArgs e)
        {
            LineWidthControl ellipseWidthControl = new LineWidthControl(LineWidthControl.LineType.TypeEllipse);
            ellipseWidthControl.OnLineWidthDialogClosed += refreshData;
            ellipseWidthControl.show();
        }

        private void mEllipseColorBtn_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ColorPickerControl ellipseColorPicker = new ColorPickerControl(ColorPickerControl.ColorType.TypeEllipse);
            ellipseColorPicker.OnColorPickerClosed += refreshData;
            ellipseColorPicker.show();
        }

        private void OnViewModeDialogClose(int mode)
        {
            switch (mode)
            {
                case 0:
                    //Vert
                    mViewModeLabel.Text = "Vertical Mode";
                    break;
                case 1:
                    //Horz
                    mViewModeLabel.Text = "Horizontal Mode";
                    break;
                case 2:
                    //Dual Page
                    mViewModeLabel.Text = "Dual Page Mode";
                    break;
            }

            if (OnViewModeSelected != null)
                OnViewModeSelected(mode);
        }

        private void RadioGroupPopup_Loaded(object sender, RoutedEventArgs e)
        {
            Windows.UI.Core.CoreWindow rcWindow = Windows.UI.Xaml.Window.Current.CoreWindow;
            Windows.Foundation.Rect rcScreen = rcWindow.Bounds;
            RadioGroupPopup.HorizontalOffset = rcScreen.Width / 2 - 250;
            RadioGroupPopup.VerticalOffset = rcScreen.Height / 2 - 150;
        }
    }
}
