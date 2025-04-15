using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace PDFViewerSDK_Win10.OptionPanelControls
{
    public delegate void OnButtonTappedHandler(int btnCode, string searchKey, bool matchCase, bool matchWholeWord);

    public sealed partial class SearchControl : UserControl
    {
        public event OnButtonTappedHandler OnButtonTapped;
        public SearchControl()
        {
            this.InitializeComponent();
        }

        public void setFocus()
        {
            searchTextBox.Focus(FocusState.Programmatic);
        }

        public string getKey() {
            return searchTextBox.Text;
        }

        public bool getMatchCase() {
            return match_case_check_box.IsChecked.Value;
        }

        public bool getMatchWholeWord()
        {
            return whole_world_check_box.IsChecked.Value;
        }

        public void enableUI(bool enabled) {
            match_case_check_box.IsEnabled = enabled;
            whole_world_check_box.IsEnabled = enabled;
        }

        private void BtnTapped(object sender, TappedRoutedEventArgs e)
        {
            if (searchTextBox.Text.Length == 0)
            {
                searchCancelBtn.IsEnabled = false;
                OnButtonTapped(-1, searchTextBox.Text, match_case_check_box.IsChecked.Value, whole_world_check_box.IsChecked.Value);
                return;
            }
            Button button = sender as Button;
            switch (button.Name)
            {
                case "searchPrevBtn":
                    searchCancelBtn.IsEnabled = true;
                    match_case_check_box.IsEnabled = false;
                    whole_world_check_box.IsEnabled = false;
                    OnButtonTapped(0, searchTextBox.Text, match_case_check_box.IsChecked.Value, whole_world_check_box.IsChecked.Value);
                    break;
                case "searchNextBtn":
                    searchCancelBtn.IsEnabled = true;
                    match_case_check_box.IsEnabled = false;
                    whole_world_check_box.IsEnabled = false;
                    OnButtonTapped(1, searchTextBox.Text, match_case_check_box.IsChecked.Value, whole_world_check_box.IsChecked.Value);
                    break;
                case "searchCancelBtn":
                    searchCancelBtn.IsEnabled = false;
                    match_case_check_box.IsEnabled = true;
                    whole_world_check_box.IsEnabled = true;
                    OnButtonTapped(-1, searchTextBox.Text, match_case_check_box.IsChecked.Value, whole_world_check_box.IsChecked.Value);
                    break;
            }
        }
    }
}
