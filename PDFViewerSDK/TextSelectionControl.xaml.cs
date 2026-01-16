using Windows.Foundation;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace PDFViewerSDK_Win10
{
    public delegate void OnCloseSelectDialogHandler(string selection, int selectType, bool cancel);

    public sealed partial class TextSelectionControl : UserControl
    {
        private int mSelectType;
        public event OnCloseSelectDialogHandler OnCloseDialog;
        private string mSelection;
        public TextSelectionControl(string selection)
        {
            this.InitializeComponent();
            mSelection = selection;
        }

        public void show()
        {
            if (!TextSelectPopup.IsOpen)
            {
                TextSelectPopup.IsOpen = true;
            }
        }

        public void dismiss()
        {
            if (TextSelectPopup.IsOpen)
            {
                TextSelectPopup.IsOpen = false;
            }
        }

        private void onButtonClick(object sender, RoutedEventArgs e)
        {
            Button button = (Button)sender;
            if (button.Name.Equals("btnOK"))
                OnCloseDialog(mSelection, mSelectType, false);
            else
                OnCloseDialog(mSelection, mSelectType, true);
            dismiss();
        }

        private void radioChecked(object sender, RoutedEventArgs e)
        {
            RadioButton selectedRadio = sender as RadioButton;
            if (selectedRadio.Name.Equals(mHighlightRadio.Name))
                mSelectType = 0;
            else if (selectedRadio.Name.Equals(mUnderlineRadio.Name))
                mSelectType = 1;
            else if (selectedRadio.Name.Equals(mStrikeoutRadio.Name))
                mSelectType = 2;
            else if (selectedRadio.Name.Equals(mClipboardRadio.Name))
                mSelectType = 3;
            else if (selectedRadio.Name.Equals(mSquigglyRadio.Name))
                mSelectType = 4;
        }

        private void TextSelectPopup_Loaded(object sender, RoutedEventArgs e)
        {
            Windows.UI.Core.CoreWindow rcWindow = Window.Current.CoreWindow;
            Rect rcScreen = rcWindow.Bounds;
            TextSelectPopup.HorizontalOffset = rcScreen.Width / 2 - 200;
            TextSelectPopup.VerticalOffset = rcScreen.Height / 2 - 150;
        }
    }
}