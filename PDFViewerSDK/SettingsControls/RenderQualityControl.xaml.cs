using PDFViewerSDK_Win10.view;
using RDPDFLib.pdf;
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
    public delegate void OnDialogClosedHandler();

    public sealed partial class RenderQualityControl : UserControl
    {
        private bool mInitialized;
        private string mSettingLabel;

        public event OnDialogClosedHandler OnDialogClosed;

        public RenderQualityControl()
        {
            this.InitializeComponent();
            mInitialized = false;
            InitializeComponent();
            mSettingLabel = "Render_quality";
            if (PDFView.renderQuality == (PDF_RENDER_MODE.mode_best))
                mBestRadio.IsChecked = true;
            else if (PDFView.renderQuality == (PDF_RENDER_MODE.mode_normal))
                mBalanceRadio.IsChecked = true;
            else if (PDFView.renderQuality == (PDF_RENDER_MODE.mode_poor))
                mFastestRadio.IsChecked = true;
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
            if (OnDialogClosed != null)
                OnDialogClosed();
            if (RadioGroupPopup.IsOpen)
            {
                //OnDialogDismissed();
                RadioGroupPopup.IsOpen = false;
            }
        }

        void radioChecked(Object sender, RoutedEventArgs e)
        {
            if (!mInitialized)
                return;
            int value = 1;
            RadioButton selectedRadio = sender as RadioButton;
            if (selectedRadio.Name.Equals(mBestRadio.Name))
            {
                value = 2;
                PDFView.renderQuality = PDF_RENDER_MODE.mode_best;
            }
            else if (selectedRadio.Name.Equals(mBalanceRadio.Name))
            {
                value = 1;
                PDFView.renderQuality = PDF_RENDER_MODE.mode_normal;
            }
            else if (selectedRadio.Name.Equals(mFastestRadio.Name))
            {
                value = 0;
                PDFView.renderQuality = PDF_RENDER_MODE.mode_poor;
            }
            Windows.Storage.ApplicationData.Current.LocalSettings.Values[mSettingLabel] = value;
            dismiss();
        }

        void RadioGroupPopup_Loaded(Object sender, RoutedEventArgs e)
        {
            Windows.UI.Core.CoreWindow rcWindow = Window.Current.CoreWindow;
            Rect rcScreen = rcWindow.Bounds;
            RadioGroupPopup.HorizontalOffset = rcScreen.Width / 2 - 200;
            RadioGroupPopup.VerticalOffset = rcScreen.Height / 2 - 150;
        }
    }
}
