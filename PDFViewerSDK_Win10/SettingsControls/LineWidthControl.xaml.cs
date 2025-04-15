using PDFViewerSDK_Win10.view;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace PDFViewerSDK_Win10.SettingsControls
{
    public delegate void OnLineWidthDialogClosedHandler();

    public sealed partial class LineWidthControl : UserControl
    {
        public enum LineType
        {
            TypeInk,
            TypeRect,
            TypeEllipse
        }

        private LineType mType;
        private string mSettingLabel;
        private bool mInitialized = false;

        public event OnLineWidthDialogClosedHandler OnLineWidthDialogClosed;

        public LineWidthControl(LineType type)
        {
            this.InitializeComponent();
            mType = type;
            float value = 1;
            switch (mType)
            {
                case LineType.TypeInk:
                    mSettingLabel = "Ink_width";
                    value = PDFView.inkWidth;
                    break;
                case LineType.TypeRect:
                    mSettingLabel = "Rect_width";
                    value = PDFView.rectWidth;
                    break;
                case LineType.TypeEllipse:
                    mSettingLabel = "Ellipse_width";
                    value = PDFView.ovalWidth;
                    break;
            }

            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey(mSettingLabel))
                value = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values[mSettingLabel]);
            else
                Windows.Storage.ApplicationData.Current.LocalSettings.Values.Add(mSettingLabel, value);

            if (value == 1)
                mFirstRadio.IsChecked = true;
            else if (value == 2)
                mSecondRadio.IsChecked = true;
            else if (value == 4)
                mThirdRadio.IsChecked = true;
            else if (value == 8)
                mFourthRadio.IsChecked = true;

            mInitialized = true;
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
            if (OnLineWidthDialogClosed != null)
                OnLineWidthDialogClosed();
            if (RadioGroupPopup.IsOpen)
            {
                RadioGroupPopup.IsOpen = false;
            }
        }

        void radioChecked(Object sender, RoutedEventArgs e)
        {
            if (!mInitialized)
                return;
            int value = 1;
            RadioButton selectedRadio = sender as RadioButton;
            if (selectedRadio.Name.Equals(mFirstRadio.Name))
            {
                value = 1;
            }
            else if (selectedRadio.Name.Equals(mSecondRadio.Name))
            {
                value = 2;
            }
            else if (selectedRadio.Name.Equals(mThirdRadio.Name))
            {
                value = 4;
            }
            else if (selectedRadio.Name.Equals(mFourthRadio.Name))
            {
                value = 8;
            }
            Windows.Storage.ApplicationData.Current.LocalSettings.Values[mSettingLabel] = value;

            switch (mType)
            {
                case LineType.TypeInk:
                    PDFView.inkWidth = value;
                    break;
                case LineType.TypeRect:
                    PDFView.rectWidth = value;
                    break;
                case LineType.TypeEllipse:
                    PDFView.ovalWidth = value;
                    break;
            }
            dismiss();
        }

        void RadioGroupPopup_Loaded(Object sender, RoutedEventArgs e)
        {
            Windows.UI.Core.CoreWindow rcWindow = Windows.UI.Xaml.Window.Current.CoreWindow;
            Rect rcScreen = rcWindow.Bounds;
            RadioGroupPopup.HorizontalOffset = rcScreen.Width / 2 - 200;
            RadioGroupPopup.VerticalOffset = rcScreen.Height / 2 - 150;
        }
    }
}
