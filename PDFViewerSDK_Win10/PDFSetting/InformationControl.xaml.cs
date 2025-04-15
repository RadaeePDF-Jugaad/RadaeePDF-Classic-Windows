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

namespace RDPDFSetting.ui
{
    public sealed partial class InformationControl : UserControl
    {
        public InformationControl()
        {
            this.InitializeComponent();
        }

        public void show()
        {
            if (!InformationPopup.IsOpen)
            {
                InformationPopup.IsOpen = true;
            }
        }

        public void dismiss()
        {
            if (InformationPopup.IsOpen)
            {
                InformationPopup.IsOpen = false;
            }
        }

        public bool isShown()
        {
            return InformationPopup.IsOpen;
        }

        //private void onButtonClick(Object sender, RoutedEventArgs e)
        //{
        //    dismiss();
        //}

        private void InformationPopup_Loaded(Object sender, RoutedEventArgs e)
        {
            Windows.UI.Core.CoreWindow rcWindow = Windows.UI.Xaml.Window.Current.CoreWindow;
            Rect rcScreen = rcWindow.Bounds;
            InformationPopup.HorizontalOffset = rcScreen.Width / 2 - 400;
            InformationPopup.VerticalOffset = rcScreen.Height / 2 - 250;
        }
    }
}
