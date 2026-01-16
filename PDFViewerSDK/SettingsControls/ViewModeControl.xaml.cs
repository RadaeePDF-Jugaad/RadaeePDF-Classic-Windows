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
    public delegate void OnViewModeDialogCloseHandler(int mode);
    public sealed partial class ViewModeControl : UserControl
    {
        public event OnViewModeDialogCloseHandler OnViewModeDialogClose;

        private bool mInitialized;
        private const string mSettingLabel = "View_mode";

        public ViewModeControl()
        {
            this.InitializeComponent();
            mInitialized = false;
            int viewMode;
            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey(mSettingLabel))
            {
                viewMode = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values[mSettingLabel]);
            }
            else
            {
                Windows.Storage.ApplicationData.Current.LocalSettings.Values.Add(mSettingLabel, 0);
                viewMode = 0;
            }
            switch (viewMode)
            {
                case 0:
                    mVerRadio.IsChecked = true;
                    break;
                case 1:
                    mHorzRadio.IsChecked = true;
                    break;
                case 2:
                    mDualPageRadio.IsChecked = true;
                    break;
                case 3:
                    mSinglePageRadio.IsChecked = true;
                    break;
                default:
                    break;
            }
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
            if (RadioGroupPopup.IsOpen)
            {
                RadioGroupPopup.IsOpen = false;
            }
        }

        private void radioChecked(Object sender, RoutedEventArgs e)
        {
            if (!mInitialized)
                return;
            int value = 0;
            RadioButton selectedRadio = sender as RadioButton;
            if (selectedRadio.Name.Equals(mVerRadio.Name))
            {
                value = 0;
            }
            else if (selectedRadio.Name.Equals(mHorzRadio.Name))
            {
                value = 1;
            }

            else if (selectedRadio.Name.Equals(mDualPageRadio.Name))
            {
                value = 2;
            }

            else if (selectedRadio.Name.Equals(mSinglePageRadio.Name))
            {
                value = 3;
            }

            if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey(mSettingLabel))
            {
                Windows.Storage.ApplicationData.Current.LocalSettings.Values[mSettingLabel] = value;
            }
            else
            {
                Windows.Storage.ApplicationData.Current.LocalSettings.Values.Add(mSettingLabel, value);
            }
            PDFView.viewMode = value;
            OnViewModeDialogClose(value);
            dismiss();
        }


        private void RadioGroupPopup_Loaded(Object sender, RoutedEventArgs e)
        {
            Windows.UI.Core.CoreWindow rcWindow = Windows.UI.Xaml.Window.Current.CoreWindow;
            Windows.Foundation.Rect rcScreen = rcWindow.Bounds;
            RadioGroupPopup.HorizontalOffset = rcScreen.Width / 2 - 200;
            RadioGroupPopup.VerticalOffset = rcScreen.Height / 2 - 150;
        }
    }
}
