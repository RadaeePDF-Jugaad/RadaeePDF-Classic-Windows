#pragma once
#include "PDFCore.h"
namespace RDPDFLib
{
	namespace view
	{
		class OPItem
		{
		protected:
			OPItem(int pgno, int idx)
			{
				m_pageno = pgno;
				m_idx = idx;
			}
			int m_idx;
		public:
			int m_pageno;
			virtual void op_undo(pdf::PDFDoc ^doc) = 0;
			virtual void op_redo(pdf::PDFDoc ^doc) = 0;
		};

		class OPDel : public OPItem
		{
		private:
			pdf::PDFRef hand;
		public:
			OPDel(int pgno, pdf::PDFPage ^page, int idx)
				:OPItem(pgno, idx)
			{
				hand = page->GetAnnot(idx)->Advance_GetRef();
			}
			void op_undo(pdf::PDFDoc ^doc)
			{
				pdf::PDFPage ^page = doc->GetPage(m_pageno);
				page->ObjsStart();
				page->AddAnnot(hand);
				page->Close();
			}

			void op_redo(pdf::PDFDoc ^doc)
			{
				pdf::PDFPage ^page = doc->GetPage(m_pageno);
				page->ObjsStart();
				pdf::PDFAnnot ^annot = page->GetAnnot(m_idx);
				annot->RemoveFromPage();
				page->Close();
			}
		};

		class OPAdd : public OPItem
		{
		private:
			pdf::PDFRef hand;
		public:
			OPAdd(int pgno, pdf::PDFPage ^page, int idx)
				:OPItem(pgno, idx)
			{
				hand = page->GetAnnot(idx)->Advance_GetRef();
			}
			void op_undo(pdf::PDFDoc ^doc)
			{
				pdf::PDFPage ^page = doc->GetPage(m_pageno);
				page->ObjsStart();
				pdf::PDFAnnot ^annot = page->GetAnnot(m_idx);
				annot->RemoveFromPage();
				page->Close();
			}

			void op_redo(pdf::PDFDoc ^doc)
			{
				pdf::PDFPage ^page = doc->GetPage(m_pageno);
				page->ObjsStart();
				page->AddAnnot(hand);
				page->Close();
			}
		};

		class OPMove : public OPItem
		{
		private:
			int m_pageno0;
			int m_pageno1;
			pdf::PDFRect m_rect0;
			pdf::PDFRect m_rect1;
		public:
			OPMove(int src_pageno, const pdf::PDFRect &src_rect, int dst_pageno, int dst_idx, const pdf::PDFRect &dst_rect)
				:OPItem(-1, dst_idx)
			{
				m_pageno0 = src_pageno;
				m_rect0 = src_rect;

				m_pageno1 = dst_pageno;
				m_rect1 = dst_rect;
			}
			void op_undo(pdf::PDFDoc ^doc)
			{
				m_pageno = m_pageno0;
				if (m_pageno == m_pageno1)
				{
					pdf::PDFPage ^page = doc->GetPage(m_pageno);
					page->ObjsStart();
					pdf::PDFAnnot ^annot = page->GetAnnot(m_idx);
					annot->Rect = m_rect0;
					page->Close();
				}
				else
				{
					pdf::PDFPage ^page0 = doc->GetPage(m_pageno0);
					pdf::PDFPage ^page1 = doc->GetPage(m_pageno1);
					page1->ObjsStart();
					page0->ObjsStart();
					pdf::PDFAnnot ^annot = page1->GetAnnot(m_idx);
					annot->MoveToPage(page0, m_rect0);
					m_idx = page1->AnnotCount;
					page0->Close();
					page1->Close();
				}
			}

			void op_redo(pdf::PDFDoc ^doc)
			{
				m_pageno = m_pageno1;
				if (m_pageno == m_pageno1)
				{
					pdf::PDFPage ^page = doc->GetPage(m_pageno);
					page->ObjsStart();
					pdf::PDFAnnot ^annot = page->GetAnnot(m_idx);
					annot->Rect = m_rect1;
					page->Close();
				}
				else
				{
					pdf::PDFPage ^page0 = doc->GetPage(m_pageno0);
					pdf::PDFPage ^page1 = doc->GetPage(m_pageno1);
					page1->ObjsStart();
					page0->ObjsStart();
					pdf::PDFAnnot ^annot = page0->GetAnnot(page0->AnnotCount - 1);
					annot->MoveToPage(page1, m_rect1);
					page0->Close();
					page1->Close();
				}
			}
		};

		class DXOPStack
		{
		private:
			OPItem **m_stack;
			int m_pos;
			int m_cnt;
			int m_max;
		public:
			DXOPStack()
			{
				m_stack = NULL;
				m_pos = -1;
				m_cnt = 0;
				m_max = 0;
			}
			~DXOPStack()
			{
				OPItem **cur = m_stack;
				OPItem **end = m_stack + m_cnt;
				while (cur < end)
				{
					delete *cur++;
				}
				free(m_stack);
				m_stack = NULL;
				m_pos = -1;
				m_cnt = 0;
				m_max = 0;
			}
			void push(OPItem *op)
			{
				m_pos++;
				if (m_pos < m_cnt)
				{
					OPItem **cur = m_stack + m_pos;
					OPItem **end = m_stack + m_cnt;
					while (cur < end)
					{
						delete *cur++;
					}
				}
				m_cnt = m_pos + 1;
				if (m_pos >= m_max)
				{
					m_max += 32;
					m_stack = (OPItem **)realloc(m_stack, sizeof(OPItem *) * m_max);
				}
				m_stack[m_pos] = op;
			}
			inline OPItem *undo()
			{
				if (m_pos < 0) return NULL;
				OPItem *ret = m_stack[m_pos--];
				return ret;
			}
			inline OPItem *redo()
			{
				if (m_pos > m_cnt - 2) return NULL;
				m_pos++;
				OPItem *ret = m_stack[m_pos];
				return ret;
			}
			inline bool can_undo()
			{
				return (m_pos >= 0);
			}
			inline bool can_redo()
			{
				return m_pos < m_cnt - 1;
			}
		};
	}
}
