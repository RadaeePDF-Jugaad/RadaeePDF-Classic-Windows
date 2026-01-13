# RadaeePDF SDK Classic for Windows (UWP)

RadaeePDF SDK is a powerful, native PDF rendering and manipulation library for Android applications. Built from true native C++ code, it provides exceptional performance and a comprehensive set of features for working with PDF documents.

## About RadaeePDF

RadaeePDF SDK is designed to solve most developers' needs with regards to PDF rendering and manipulation. The SDK is trusted across industries worldwide including automotive, banking, publishing, healthcare, and more.

### Key Features

- **PDF ISO32000 Compliance** - Full support for the widely-used PDF format standard
- **High Performance** - True native code compiled from C++ sources for optimal speed
- **Annotations** - Create and manage text annotations, highlights, ink annotations, and more
- **Protection & Encryption** - Full AES256 cryptography for document security
- **Text Handling** - Search, extract, and highlight text with ease
- **Form Editing** - Create, read, and write PDF form fields (AcroForms)
- **Digital Signatures** - Sign and verify PDF documents with digital certificates
- **Multiple View Modes** - Single page, continuous scroll, and more
- **Night Mode** - Built-in dark mode support for better readability

### Manual Installation

1. Download the RadaeePDF SDK library from Git Repository

## Getting Started

### Initialize the Library

Before using RadaeePDF, initialize the library with your license key:

1. Modify the 'PDFGlobal.cs' file under namespace 'com.radaee.master', to add your license key:
```csharp
static public bool init()
{
    load_data();
    int licenseType = RDPDFLib.pdf.RDGlobal.Active("[YOUR-LICENSE-KEY]");
    return (licenseType > 0);
}
```

2. Add the following code to your 'App' class to initialize and activate the library:
```csharp
using com.radaee.master;

sealed partial class App : Application
{
    protected override void OnLaunched(LaunchActivatedEventArgs e)
    {
        //...... your code
        PDFGlobal.init();
        //...... your code
    }
}
```

### Open and Display a PDF

```csharp
using com.radaee.master;
using RDDLib.comm;
using RDDLib.pdf;
using RDDLib.pdfv;
using RDDLib.reader;
using RDPDFMaster;
using RDPDFMaster.Modules.FTS;
using RDPDFMaster.PDFReaderPage;
using RDPDFMaster.util;
using RDPDFReader.annotui;

public sealed partial class PDFReaderPage : Page, IPDFViewListener, IThumbListener
{
    private PDFReader m_reader;
    private PDFThumb m_thumb;
    private PDFDoc m_doc;
    private void file_open(IRandomAccessStream stream, String password)
    {
        m_doc = new PDFDoc();
        RD_ERROR err = m_doc.Open(stream, password);
        switch (err)
        {
            case RD_ERROR.err_open:
                //can't open the PDF file
                break;
            case RD_ERROR.err_invalid_para:
                //invalid parameter
                break;
            case RD_ERROR.err_encrypt:
                //PDF file encrypted with unknown filter.
                break;
            case RD_ERROR.err_bad_file:
                //bad file
                break;
            case RD_ERROR.err_password:
                //password required
                break;
            case RD_ERROR.err_ok:
                //success
                m_reader = new PDFReader();

                //mView: RelativePanel control in XAML
                m_reader.PDFOpen(mView, m_doc, PDF_LAYOUT_MODE.layout_vert, this);

                //mThumb: RelativePanel control in XAML
                m_thumb = new PDFThumb();
                m_thumb.PDFOpen(mThumb, m_doc, this);
                break;
            default:
                //Unknown error
                break;
        }
    }
}
```

## Common Operations

### Get Page Count

```csharp
int pageCount = m_doc.PageCount;
```

### Navigate to a Specific Page

```csharp
m_reader.PDFGotoPage(5); // Go to page 5
```

### Set View Mode

```csharp
m_reader.PDFViewMode = PDF_LAYOUT_MODE.layout_vert; // Vertical scroll mode
```

### Text Search (Professional License)

```csharp
// Start search
boolean match_case = false;
boolean match_whole_word = false;
m_reader.PDFFindStart("search_term", match_case, match_whole_word);

// Find next/previous occurrence
int dir = 1; // 1 = next, -1 = previous
m_reader.PDFFind(dir);

// End search and reset
m_reader.PDFFindEnd();
```

### Text Highlighting (Professional License)

```csharp
uint color = 0xFFFFFF00; // Yellow color
// Highlight selected text
m_reader.PDFSelSetMarkup(0, color); // 0 = highlight

// Underline selected text
m_reader.PDFSelSetMarkup(1, color); // 1 = underline

// Strikeout selected text
m_reader.PDFSelSetMarkup(2, color); // 2 = strikeout

// Rod squiggly selected text
m_reader.PDFSelSetMarkup(4, color); // 4 = rod squiggly
```

### Add Annotations (Professional License)

```csharp
// Add a note annotation at point (x, y)
PDFPage page = m_doc.GetPage(0); // Get the first page
if(page != null) {
    page.ObjsStart();
    page.AddAnnotTextNote(x, y);
    page.Close();
}


// Remove annotation
PDFPage page = m_doc.GetPage(0); // Get the first page
if (page != null) {
    //Start object mode
    page.ObjsStart();
    PDFAnnot annot = page.GetAnnot(0);
    if (annot != null) { 
        annot.RemoveFromPage();
    }

    //Close page and release holding resource
    page.Close();
}
```

### Save Document

```csharp
// Save changes to the same file
m_doc.Save();

// Save to a new file
m_doc.SaveAs("[path_to_save]");
```

## License Levels

RadaeePDF offers different license levels with varying features:

Visit [https://www.radaeepdf.com/](https://www.radaeepdf.com/) for detailed licensing information.

## Documentation

For complete API documentation and advanced features, visit:
- [RadaeePDF Support Portal](https://support.radaeepdf.com/)
- [Wiki](https://github.com/RadaeePDF-Jugaad/RadaeePDF-Classic-Windows/wiki)

## Support

For technical support and questions:
- Email: support@radaeepdf.com
- Website: [https://www.radaeepdf.com/](https://www.radaeepdf.com/)

## License

This SDK is commercial software. Please ensure you have a valid license before using it in production applications.

---

© 2026 RadaeePDF. All rights reserved.
