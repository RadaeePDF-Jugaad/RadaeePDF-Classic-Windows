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

namespace PDFViewerSDK_Win10.OptionPanelControls
{
    public delegate void OnEditAnnotButtonClickHandler(int buttonCode);

    public sealed partial class EditAnnotControl : UserControl
    {
        public event OnEditAnnotButtonClickHandler OnButtonClick;

        public EditAnnotControl()
        {
            this.InitializeComponent();
        }

        private void onItemTapped(Object sender, TappedRoutedEventArgs e)
        {
            Button button = sender as Button;
            int buttonCode = -1;
            if (button.Equals(mOpenAnnotBtn))
            {
                buttonCode = 0;
            }
            else if (button.Equals(mRemoveAnnotBtn))
            {
                buttonCode = 1;
            }
            else if (button.Equals(mCancelAnnotBtn))
            {
                buttonCode = 2;
            }
            OnButtonClick(buttonCode);
        }
    }
}
