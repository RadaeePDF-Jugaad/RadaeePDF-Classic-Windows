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

namespace RDPDFReader.annotui
{
    public delegate void OnCloseSaveDialogHandler(int button);
    public sealed partial class SaveDocControl : UserControl
    {
        public event OnCloseSaveDialogHandler onSaveDialogClose;

        public SaveDocControl()
        {
            this.InitializeComponent();
        }

        private void SaveDocPopup_Loaded(Object sender, RoutedEventArgs e)
        {
            Windows.UI.Core.CoreWindow rcWindow = Windows.UI.Xaml.Window.Current.CoreWindow;
            Rect rcScreen = rcWindow.Bounds;
            SaveDocPopup.HorizontalOffset = rcScreen.Width / 2 - 200;
            SaveDocPopup.VerticalOffset = rcScreen.Height / 2 - 150;
        }

        private void onButtonClick(Object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            if (button.Name.Equals("btnOK"))
            {
                onSaveDialogClose(0);
            }
            else if (button.Name.Equals("btnNo"))
                onSaveDialogClose(1);
            else
                onSaveDialogClose(2);
            dismiss();
        }

        public void dismiss()
        {
            if (SaveDocPopup.IsOpen)
            {
                SaveDocPopup.IsOpen = false;
            }
        }

        public void show()
        {
            if (!SaveDocPopup.IsOpen)
            {
                SaveDocPopup.IsOpen = true;
            }
        }

        public bool isShown()
        {
            return SaveDocPopup.IsOpen;
        }
    }
}
