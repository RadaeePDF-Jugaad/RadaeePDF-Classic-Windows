#include "pch.h"
#include "PDFPanel.h"

using namespace RDPDFLib::reader;
void PDFLayoutView::vpShowBlock(Image^ img)
{
	Children->Append(img);
}

void PDFLayoutView::vpRemoveBlock(Image^ img)
{
	unsigned int idx;
	if (Children->IndexOf(img, &idx))
		Children->RemoveAt(idx);
}
