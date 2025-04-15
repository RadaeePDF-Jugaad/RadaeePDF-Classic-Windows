#include "pch.h"
#include "PDFCore.h"
#include "SHCore.h"

using namespace RDPDFLib::pdf;
/*
extern "C" char *getenv(char *envvar)
{
	return NULL;
}
*/
float PDFGlobal::zoom_level = 3;
/*
byte RDPDFLib::pdf::g_tiny_bmp[102] =
{
	0x42, 0x4D, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
	0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
*/

char *cvt_str_cstr( String ^str )
{
	if( !str ) return NULL;
	const wchar_t *wstr = str->Data();
	int wlen = str->Length();
	char *data = (char *)malloc( (wlen + 1) * 2 );
	int len = ::WideCharToMultiByte( CP_ACP, 0, wstr, wlen, data, (wlen + 1) * 2, NULL, NULL );
	data[len] = 0;
	return data;
}
String ^cvt_cstr_str( const char *str )
{
	if( !str || !str[0] ) return nullptr;
	int len1 = strlen( str ) + 2;
	wchar_t *wtxt = (wchar_t *)malloc( sizeof(wchar_t) * len1 );
	int len = MultiByteToWideChar( CP_ACP, 0, str, -1, wtxt, len1 );
	wtxt[len] = 0;
	String ^ret = ref new String( wtxt );
	free( wtxt );
	return ret;
}


/// <summary>
/// Add a PDFDocForm resource to document form, and generate a PDFPageForm base on it.
/// </summary>
/// <param name="sub">The PDFDocForm to add</param>
/// <returns>Generated PDFPageForm object</returns>
PDFPageForm ^PDFDocForm::AddResForm(PDFDocForm ^sub)
{
	PDF_PAGE_FORM hand = Document_addFormResForm(m_doc->m_doc, m_form, sub->m_form);
	if (!hand) return nullptr;
	PDFPageForm ^form = ref new PDFPageForm();
	form->m_form = hand;
	return form;
}
/// <summary>
/// Add a font resource to document form, and generate a PDFPageFont base on it.
/// </summary>
/// <param name="font">The font resource to add</param>
/// <returns>Generated PDFPageFont object</returns>
PDFPageFont ^PDFDocForm::AddResFont(PDFDocFont ^font)
{
	PDF_PAGE_FONT hand = Document_addFormResFont(m_doc->m_doc, m_form, font->m_font);
	if (!hand) return nullptr;
	PDFPageFont ^pf = ref new PDFPageFont();
	pf->m_font = hand;
	return pf;
}
/// <summary>
/// Add a graphic state to document form, and generate a PDFPageGState base on it.
/// </summary>
/// <param name="font">The graphic state to add</param>
/// <returns>Generated PDFPageGState object</returns>
PDFPageGState ^PDFDocForm::AddResGState(PDFDocGState ^gs)
{
	PDF_PAGE_GSTATE hand = Document_addFormResGState(m_doc->m_doc, m_form, gs->m_gs);
	if (!hand) return nullptr;
	PDFPageGState ^pgs = ref new PDFPageGState();
	pgs->m_gs = hand;
	return pgs;
}
/// <summary>
/// Add a image resource to document form, and generate a PDFPageImage base on it.
/// </summary>
/// <param name="font">The image resource to add</param>
/// <returns>Generated PDFPageImage object</returns>
PDFPageImage ^PDFDocForm::AddResImage(PDFDocImage ^img)
{
	PDF_PAGE_IMAGE hand = Document_addFormResImage(m_doc->m_doc, m_form, img->m_image);
	if (!hand) return nullptr;
	PDFPageImage ^pi = ref new PDFPageImage();
	pi->m_image = hand;
	return pi;
}
/// <summary>
/// Add a PDFPageContent to document form.
/// </summary>
/// <param name="content">The PDFPageContent to add</param>
/// <param name="x">The x coordinate ofthe top left point to add the content</param>
/// <param name="y">The y coordinate ofthe top left point to add the content</param>
/// <param name="w">The width of the content</param>
/// <param name="h">The height of the content</param>
void PDFDocForm::SetContent(PDFPageContent ^content, float x, float y, float w, float h)
{
	Document_setFormContent(m_doc->m_doc, m_form, x, y, w, h, content->m_content);
}

PDFDoc::PDFDoc()
{
	m_doc = NULL;
	m_inner = NULL;
}
PDFDoc::~PDFDoc()
{
	Close();
}
/// <summary>
/// Open a PDF document
/// </summary>
/// <param name="stream">A random access stream which is returned by opening a local file</param>
/// <param name="password">Password if the document is a password protected document</param>
/// <returns>A PDF_ERROR which indicate the result of opening the document. Please refer to definition of PDF_ERROR class for more details</returns>
PDF_ERROR PDFDoc::Open( IRandomAccessStream ^stream, String ^password )
{
	PDF_ERR err;
	char *pswd = cvt_str_cstr( password );
	m_doc = Document_open( stream, pswd, &err );
	free( pswd );
	if( m_doc ) return PDF_ERROR::err_ok;
	else return (PDF_ERROR)err;
}

/// <summary>
/// Open a PDF document in stream mode
/// </summary>
/// <param name="stream">A PDFStream which provides the PDF document content</param>
/// <param name="password">Password if the document is a password protected document</param>
/// <returns>A PDF_ERROR which indicate the result of opening the document. Please refer to definition of PDF_ERROR class for more details</returns>
PDF_ERROR PDFDoc::OpenStream( PDFStream ^stream, String ^password )
{
	if (!stream) return PDF_ERROR::err_invalid_para;
	PDF_ERR err;
	char *pswd = cvt_str_cstr( password );
	m_inner = new PDFStreamInner;
	m_inner->Open( stream );
	m_doc = Document_openStream( m_inner, pswd, &err );
	free( pswd );
	if( m_doc ) return PDF_ERROR::err_ok;
	else return (PDF_ERROR)err;
}

/// <summary>
/// Open a PDF document from specified path
/// </summary>
/// <param name="path">The file path</param>
/// <param name="password">Password if the document is a password protected document</param>
/// <returns>A PDF_ERROR which indicate the result of opening the document. Please refer to definition of PDF_ERROR class for more details</returns>
PDF_ERROR PDFDoc::OpenPath( String ^path, String ^password )
{
	if (!path) return PDF_ERROR::err_invalid_para;
	PDF_ERR err;
	char *pswd = cvt_str_cstr( password );
	m_doc = Document_openPathW( path->Data(), pswd, &err );
	free( pswd );
	if( m_doc ) return PDF_ERROR::err_ok;
	else return (PDF_ERROR)err;
}
/// <summary>
/// Get document embedded meta data with specified tag
/// </summary>
/// <param name="tag">Tag of target meta data</param>
/// <returns>The content of the meta data. Empty string if corresponding meta data is not found</returns>
String ^PDFDoc::GetMeta(String ^tag)
{
	if( !tag ) return nullptr;
	char *ctag = cvt_str_cstr( tag );
	int wlen = 1024;
	String^ meta = Document_getMeta(m_doc, ctag);
	free( ctag );
	return meta;
}
/// <summary>
/// Set content to meta data with specified tag
/// </summary>
/// <param name="tag">Tag of the meta data</param>
/// <param name="val">Value of the meta data</param>
/// <returns>True if succssed, otherwise false</returns>
bool PDFDoc::SetMeta(String^ tag, String^ val)
{
	if (!tag || !val) return false;
	char* ctag = cvt_str_cstr(tag);
	bool ret = Document_setMeta(m_doc, ctag, val->Data());
	free(ctag);
	return ret;
}

/// <summary>
/// Export PDF form as string
/// </summary>
/// <returns>Generated string</returns>
String ^PDFDoc::ExportForm()
{
	char txt[2048];
	if( !Document_exportForm( m_doc, txt, 2048 ) ) return nullptr;
	wchar_t wtxt[2048];
	::MultiByteToWideChar( CP_UTF8, 0, txt, -1, wtxt, 2047 );
	return ref new String(wtxt);
}

/// <summary>
/// Get a PDFPage object with specified page number
/// </summary>
/// <param name="pageno">Page number of target page</param>
/// <returns>PDFPage object of the PDF page</returns>
PDFPage ^PDFDoc::GetPage(int pageno)
{
	PDF_PAGE page = Document_getPage( m_doc, pageno );
	if( page )
	{
		PDFPage ^pg = ref new PDFPage();
		pg->m_page = page;
		//pg->m_doc = this;
		return pg;
	}
	else
		return nullptr;
}

/// <summary>
/// Get the root node of embedded PDF outline
/// </summary>
/// <returns>A PDFOutline object represents the root of the outline</returns>
PDFOutline ^PDFDoc::GetRootOutline()
{
	PDF_OUTLINE outline = Document_getOutlineNext( m_doc, NULL );
	if( outline )
	{
		PDFOutline ^otl =  ref new PDFOutline();
		otl->m_doc = this;
		otl->m_outline = outline;
		return otl;
	}
	else return nullptr;
}

/// <summary>
/// Add a root node to embedded PDF outline
/// </summary>
/// <param name="label">Label of the node</param>
/// <param name="dest">The destination which the node points at</param>
/// <param name="y"></param>
/// <returns></returns>
Boolean PDFDoc::AddRootOutline( String ^label, int dest, float y )
{
	return Document_addOutlineNext( m_doc, NULL, label->Data(), dest, y );
}
/// <summary>
/// Save changes to the document.
/// </summary>
/// <returns>True if successed, otherwise false</returns>
Boolean PDFDoc::Save()
{
	return Document_save(m_doc);
}

/// <summary>
/// Close opened document
/// </summary>
void PDFDoc::Close()
{
	Document_close(m_doc);
	m_doc = NULL;
	if( m_inner )
	{
		m_inner->Close();
		delete m_inner;
	}
	m_inner = NULL;
}

/// <summary>
/// Get specified page width.
/// </summary>
/// <param name="pageno">The page number of target page</param>
/// <returns>The width of the PDF page.</returns>
float PDFDoc::GetPageWidth(int pageno)
{
	return Document_getPageWidth( m_doc, pageno );
}
/// <summary>
/// Get specified page height.
/// </summary>
/// <param name="pageno">The page number of target page</param>
/// <returns>The height of the PDF page.</returns>
float PDFDoc::GetPageHeight(int pageno)
{
	return Document_getPageHeight( m_doc, pageno );
}
/// <summary>
/// Create a new PDFDocImage resource with provided WriteableBitmap
/// </summary>
/// <param name="bitmap">The WriteableBitmap object which will be used to create the PDFDocImage resource</param>
/// <param name="has_alpha">True if the PDFDocImage should has alpha value, only available the WriteableBitmap resource has alpha property.</param>
/// <returns>The generated PDFDocImage</returns>
PDFDocImage ^PDFDoc::NewImage(WriteableBitmap ^bitmap, bool has_alpha)
{
	PDF_DOC_IMAGE image = Document_newImage(m_doc, bitmap, has_alpha);
	if (image)
	{
		PDFDocImage ^img = ref new PDFDocImage();
		img->m_image = image;
		return img;
	}
	else return nullptr;
}
/// <summary>
/// Create a new PDFDocImage resource with provided SoftwareBitmap
/// </summary>
/// <param name="bitmap">The SoftwareBitmap object which will be used to create the PDFDocImage resource</param>
/// <param name="has_alpha">True if the PDFDocImage should has alpha value, only available the SoftwareBitmap resource has alpha property.</param>
/// <returns>The generated PDFDocImage</returns>
PDFDocImage^ PDFDoc::NewImage(SoftwareBitmap^ bitmap, bool has_alpha)
{
	PDF_DOC_IMAGE image = Document_newImage2(m_doc, bitmap, has_alpha);
	if (image)
	{
		PDFDocImage^ img = ref new PDFDocImage();
		img->m_image = image;
		return img;
	}
	else return nullptr;
}

/// <summary>
/// Create a new PDFDocImage resource with provided WriteableBitmap
/// </summary>
/// <param name="bitmap">The WriteableBitmap object which will be used to create the PDFDocImage resource</param>
/// <param name="matte">A color which will be used to calculate the background color with alpha value</param>
/// <returns>The generated PDFDocImage</returns>
PDFDocImage^ PDFDoc::NewImage(WriteableBitmap^ bitmap, unsigned int matte)
{
	PDF_DOC_IMAGE image = Document_newImageMatte(m_doc, bitmap, matte);
	if (image)
	{
		PDFDocImage^ img = ref new PDFDocImage();
		img->m_image = image;
		return img;
	}
	else return nullptr;
}

/// <summary>
/// Create a new PDFDocImage resource with provided SoftwareBitmap
/// </summary>
/// <param name="bitmap">The SoftwareBitmap object which will be used to create the PDFDocImage resource</param>
/// <param name="matte">A color which will be used to calculate the background color with alpha value</param>
/// <returns>The generated PDFDocImage</returns>
PDFDocImage^ PDFDoc::NewImage(SoftwareBitmap^ bitmap, unsigned int matte)
{
	PDF_DOC_IMAGE image = Document_newImage2Matte(m_doc, bitmap, matte);
	if (image)
	{
		PDFDocImage^ img = ref new PDFDocImage();
		img->m_image = image;
		return img;
	}
	else return nullptr;
}

/// <summary>
/// Create a PDFDocImage with JEPG image file at specified path
/// </summary>
/// <param name="path">The path of JEPG image file</param>
/// <returns>The generated PDFDocImage</returns>
PDFDocImage ^PDFDoc::NewImageJPEG( String ^path )
{
	char *cpath = cvt_str_cstr( path );
	PDF_DOC_IMAGE image = Document_newImageJPEG( m_doc, cpath );
	free( cpath );
	if( image )
	{
		PDFDocImage ^img = ref new PDFDocImage();
		img->m_image = image;
		return img;
	}
	else return nullptr;
}

/// <summary>
/// Create a PDFDocImage with JPX image file at specified path
/// </summary>
/// <param name="path">The path of JPX image file</param>
/// <returns>The generated PDFDocImage</returns>
PDFDocImage ^PDFDoc::NewImageJPX( String ^path )
{
	char *cpath = cvt_str_cstr( path );
	PDF_DOC_IMAGE image = Document_newImageJPX( m_doc, cpath );
	free( cpath );
	if( image )
	{
		PDFDocImage ^img = ref new PDFDocImage();
		img->m_image = image;
		return img;
	}
	else return nullptr;
}

/// <summary>
/// Create a PDFDocFont resource with specified font name and style
/// </summary>
/// <param name="name">The name of the font</param>
/// <param name="style">The font style, bold, italic, etc..</param>
/// <returns>Generated PDFDocFont resource</returns>
PDFDocFont ^PDFDoc::NewFontCID( String ^name, int style )
{
	char *fname = cvt_str_cstr( name );
	PDF_DOC_FONT font = Document_newFontCID( m_doc, fname, style );
	free( fname );
	if( font )
	{
		PDFDocFont ^fnt = ref new PDFDocFont();
		fnt->m_font = font;
		fnt->m_doc = this;
		return fnt;
	}
	else return nullptr;
}

/// <summary>
/// Create a new PDF graphic state
/// </summary>
/// <returns></returns>
PDFDocGState ^PDFDoc::NewGState()
{
	PDF_DOC_GSTATE gs = Document_newGState( m_doc );
	if( gs )
	{
		PDFDocGState ^state = ref new PDFDocGState();
		state->m_gs = gs;
		state->m_doc = this;
		return state;
	}
	else return nullptr;
}

/// <summary>
/// Create a new PDF form
/// </summary>
/// <returns></returns>
PDFDocForm ^PDFDoc::NewForm()
{
	PDF_DOC_FORM hand = Document_newForm(m_doc);
	if (!hand) return nullptr;
	PDFDocForm ^form = ref new PDFDocForm();
	form->m_doc = this;
	form->m_form = hand;
	return form;
}

/// <summary>
/// Remove a page from PDF document
/// </summary>
/// <param name="pageno">Page number of the page which should be removed</param>
/// <returns>True if successed, otherwise false</returns>
Boolean PDFDoc::RemovePage( int pageno )
{
	return Document_removePage( m_doc, pageno );
}

/// <summary>
/// Mode a page to a new index position
/// </summary>
/// <param name="srcno">Page number of the page which should be moved</param>
/// <param name="dstno">new index position which the page will be moved to</param>
/// <returns>True if successed, otherwise false</returns>
Boolean PDFDoc::MovePage( int srcno, int dstno )
{
	return Document_movePage( m_doc, srcno, dstno );
}

/// <summary>
/// Create a new PDF page
/// </summary>
/// <param name="pageno">The index position of the new page</param>
/// <param name="w">Width of the new page</param>
/// <param name="h">Height of the new page</param>
/// <returns>The created PDFPage object</returns>
PDFPage ^PDFDoc::NewPage( int pageno, float w, float h )
{
	PDF_PAGE pg = Document_newPage( m_doc, pageno, w, h );
	if( pg )
	{
		PDFPage ^page = ref new PDFPage();
		//page->m_doc = this;
		page->m_page = pg;
		return page;
	}
	else return nullptr;
}

/// <summary>
/// Start importing from another PDF document
/// </summary>
/// <param name="src">PDF document which is source to import</param>
/// <returns>A PDF import context which is responsible for executing the importing operation</returns>
PDFImportCtx ^PDFDoc::ImportStart(PDFDoc ^src)
{
	PDF_IMPORTCTX ctx = Document_importStart( m_doc, src->m_doc );
	if( ctx )
	{
		PDFImportCtx ^ictx = ref new PDFImportCtx();
		ictx->m_doc = this;
		ictx->m_ctx = ctx;
		return ictx;
	}
	else return nullptr;
}


/// <summary>
/// Verify if a PDF signature is correct
/// </summary>
/// <param name="sign">PDF signature which will be verified</param>
/// <returns>0 if the signature is correct.</returns>
int PDFDoc::VerifySign(PDFSign ^sign)
{
	if (!sign) return -1;
	return Document_verifySign(m_doc, sign->m_sign);
}

/// <summary>
/// Get next PDF outline node
/// </summary>
/// <returns>Next PDF outline node if exist, otherwise nullptr</returns>
PDFOutline ^PDFOutline::GetNext()
{
	PDF_OUTLINE otl = Document_getOutlineNext( m_doc->m_doc, m_outline );
	if( otl )
	{
		PDFOutline ^outline = ref new PDFOutline();
		outline->m_doc = m_doc;
		outline->m_outline = otl;
		return outline;
	}
	else return nullptr;
}

/// <summary>
/// Get first child of current PDF outline node
/// </summary>
/// <returns>First child node of current PDF outline node if exist, otherwise nullptr</returns>
PDFOutline ^PDFOutline::GetChild()
{
	PDF_OUTLINE otl = Document_getOutlineChild( m_doc->m_doc, m_outline );
	if( otl )
	{
		PDFOutline ^outline = ref new PDFOutline();
		outline->m_doc = m_doc;
		outline->m_outline = otl;
		return outline;
	}
	else return nullptr;
}

/// <summary>
/// Append a PDF outline node to current
/// </summary>
/// <param name="label">Label of the new node</param>
/// <param name="dest">Destiny postion which the new node is pointing at</param>
/// <param name="y">Offset of the destiny postion</param>
/// <returns>True if successed, otherwise false</returns>
Boolean PDFOutline::AddNext( String ^label, int dest, float y )
{
	return Document_addOutlineNext( m_doc->m_doc, m_outline, label->Data(), dest, y );
}

/// <summary>
/// Add a PDF outline node as child to current
/// </summary>
/// <param name="label">Label of the new node</param>
/// <param name="dest">Destiny postion which the new node is pointing at</param>
/// <param name="y">Offset of the destiny postion</param>
/// <returns>True if successed, otherwise false</returns>
Boolean PDFOutline::AddChild( String ^label, int dest, float y )
{
	return Document_addOutlineChild( m_doc->m_doc, m_outline, label->Data(), dest, y );
}

/// <summary>
/// Remove a PDF outline node from PDF document
/// </summary>
/// <returns>True if successed, otherwise false</returns>
Boolean PDFOutline::RemoveFromDoc()
{
	bool ret = Document_removeOutline( m_doc->m_doc, m_outline );
	if( ret )
	{
		m_outline = NULL;
		m_doc = nullptr;
	}
	return ret;
}
/// <summary>
/// Get annotation at specified index
/// </summary>
/// <param name="index">0 based index of the page contents</param>
/// <returns>The PDFAnnot object if found, otherwise nullptr</returns>
PDFAnnot ^PDFPage::GetAnnot( int index )
{
	PDF_ANNOT annot = Page_getAnnot( m_page, index );
	if( annot )
	{
		PDFAnnot ^annot1 = ref new PDFAnnot();
		annot1->m_annot = annot;
		annot1->m_page = this;
		return annot1;
	}
	else return nullptr;
}

/// <summary>
/// Get annotation at specified position
/// </summary>
/// <param name="x">x coordinate of the postion in PDF page</param>
/// <param name="y">y coordinate of the postion in PDF page</param>
/// <returns>The PDFAnnot object if found, otherwise nullptr</returns>
PDFAnnot ^PDFPage::GetAnnot( float x, float y )
{
	PDF_ANNOT annot = Page_getAnnotFromPoint( m_page, x, y );
	if( annot )
	{
		PDFAnnot ^annot1 = ref new PDFAnnot();
		annot1->m_annot = annot;
		annot1->m_page = this;
		return annot1;
	}
	else return nullptr;
}

/// <summary>
/// Add a popup annotation to PDF page
/// </summary>
/// <param name="parent">The parent annotation which holds the popup</param>
/// <param name="rect">A rectangle which specifies the location and size of the popup</param>
/// <param name="open">If the popup is opened by default or not</param>
/// <returns>True if successed, otherwise false</returns>
Boolean PDFPage::AddAnnotPopup(PDFAnnot ^parent, PDFRect rect, bool open)
{
	return Page_addAnnotPopup(m_page, parent->m_annot, (const PDF_RECT *)&rect, open);
}
