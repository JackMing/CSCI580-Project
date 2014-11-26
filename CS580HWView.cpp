// CS580HWView.cpp : implementation of the CCS580HWView class
//

#include "stdafx.h"
#include "CS580HW.h"

#include "CS580HWDoc.h"
#include "CS580HWView.h"
#include "RotateDlg.h"
#include "TranslateDlg.h"
#include "ScaleDlg.h"
#include <math.h>
#include "disp.h"
#include "Application5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int counter=0;

/////////////////////////////////////////////////////////////////////////////
// CCS580HWView

IMPLEMENT_DYNCREATE(CCS580HWView, CView)

BEGIN_MESSAGE_MAP(CCS580HWView, CView)
	//{{AFX_MSG_MAP(CCS580HWView)
	ON_COMMAND(IDM_RENDER, OnRender)
	ON_COMMAND(IDM_ROTATE, OnRotate)
	ON_COMMAND(IDM_TRANSLATE, OnTranslate)
	ON_COMMAND(IDM_SCALE, OnScale)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCS580HWView construction/destruction

CCS580HWView::CCS580HWView()
{
	// TODO: add construction code here
	m_pApplication = NULL;
}

CCS580HWView::~CCS580HWView()
{
	if(m_pApplication != NULL)
	{
		delete m_pApplication;
	}
}

BOOL CCS580HWView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCS580HWView drawing

void CCS580HWView::OnDraw(CDC* pDC)
{
	CCS580HWDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
	if(m_pApplication != NULL)
		DrawFrameBuffer(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CCS580HWView diagnostics

#ifdef _DEBUG
void CCS580HWView::AssertValid() const
{
	CView::AssertValid();
}

void CCS580HWView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCS580HWDoc* CCS580HWView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCS580HWDoc)));
	return (CCS580HWDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCS580HWView message handlers

void CCS580HWView::OnRender() 
{
	// TODO: Add your command handler code here

	// Call renderer 

	// Application 5
	if(m_pApplication != NULL)
		((Application5 *)m_pApplication)->Render();
	else 
		AfxMessageBox("Application was not allocated\n");

	// Set window size
	CRect clientRect, windowRect;
	int x_offset, y_offset;

	GetClientRect(&clientRect);
	AfxGetMainWnd()->GetWindowRect(&windowRect);
	
	x_offset = windowRect.Width() - clientRect.Width();
	y_offset = windowRect.Height() - clientRect.Height();

	//AfxGetMainWnd()->SetWindowPos(NULL, 0, 0, x_offset+m_pApplication->m_nWidth, y_offset+m_pApplication->m_nHeight, NULL/*,SWP_SHOWWINDOW*/);

	Invalidate(true);	
}

void CCS580HWView::DrawFrameBuffer(CDC *pDC)
{
	if(m_pApplication->m_pFrameBuffer == NULL)
    {
        return;
    }

    HDC hdc;
    hdc = ::CreateCompatibleDC(pDC->m_hDC);
	HBITMAP m_bitmap;

    // Display the current image
    char buffer[sizeof(BITMAPINFO)];
    BITMAPINFO* binfo = (BITMAPINFO*)buffer;
    memset(binfo, 0, sizeof(BITMAPINFOHEADER));
    binfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    
	// Create the bitmap
    BITMAPINFOHEADER* bih = &binfo->bmiHeader;
	bih->biBitCount = 3*8;
	bih->biWidth = m_pApplication->m_nWidth;
	bih->biHeight = m_pApplication->m_nHeight;
    bih->biPlanes = 1;
    bih->biCompression = BI_RGB;
    bih->biSizeImage = 0;
    
    m_bitmap = CreateDIBSection(hdc, binfo, 0, 0, 0, DIB_RGB_COLORS);

    int colors = DIB_RGB_COLORS;
    
    ::SelectObject(hdc, m_bitmap);
	binfo->bmiHeader.biBitCount = 0;
	GetDIBits(hdc, m_bitmap, 0, 0, 0, binfo, colors);
    binfo->bmiHeader.biBitCount = 24;
    binfo->bmiHeader.biHeight = -abs(binfo->bmiHeader.biHeight);
    SetDIBits(hdc, m_bitmap, 0, m_pApplication->m_nHeight, m_pApplication->m_pFrameBuffer, binfo, colors);

    ::SetStretchBltMode(pDC->m_hDC, COLORONCOLOR);
    CRect client;
    GetClientRect(&client);
    ::BitBlt(pDC->m_hDC, 0, 0, m_pApplication->m_nWidth, m_pApplication->m_nHeight, 
        hdc, 0, 0, SRCCOPY);
    ::DeleteDC(hdc);
    DeleteObject(m_bitmap); 
}

void CCS580HWView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class

	// Assign Application 5
	if(m_pApplication == NULL)
	{
		m_pApplication = new Application5;
	}
	
	// Initialize and begin renderer
	((Application5 *)m_pApplication)->Initialize();
}

// Callback function for rotation  
void CCS580HWView::OnRotate() 
{
	// TODO: Add your command handler code here
	CRotateDlg dlg;
	GzInput* input;
	GzMatrix	rotMat = 
	{ 
		1.0,	0.0,	0.0,	0.0, 
		0.0,	1.0,	0.0,	0.0, 
		0.0,	0.0,	1.0,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	};


	if(m_pApplication == NULL) return;

	input = m_pApplication->m_pUserInput;
	if(input == NULL) return;

	// Initialize
	input->rotation[0] = input->rotation[1] = input->rotation[2] = 0;
	dlg.Initialize(input->rotation[0], input->rotation[1], input->rotation[2]);

	if(dlg.DoModal() == IDOK)
	{
		// Update input rotation value
		input->rotation[dlg.m_nAxis] = dlg.m_fRot;
				
		//  Create Rotation Matrix 
		switch(dlg.m_nAxis)
		{
		case 0 :
			// Create matrix for Rot X
			GzRotXMat(input->rotation[0], rotMat);
			break;
		case 1:
			// Create matrix for Rot Y
			GzRotYMat(input->rotation[1], rotMat);
			break;
		case 2:
			// Create matrix for Rot Z
			GzRotZMat(input->rotation[2], rotMat);
			break;
		}

		// Accumulate matrix
		for(int i=0;i<AAKERNEL_SIZE;i++)
			GzPushMatrix(m_pApplication->m_pRender[i], rotMat); 
	}
}

// Callback function for Translation
void CCS580HWView::OnTranslate() 
{
	// TODO: Add your command handler code here
	CTranslateDlg dlg;
	GzInput* input;
	GzMatrix	trxMat = 
	{ 
		1.0,	0.0,	0.0,	0.0, 
		0.0,	1.0,	0.0,	0.0, 
		0.0,	0.0,	1.0,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	};


	if(m_pApplication == NULL) return;

	input = m_pApplication->m_pUserInput;
	if(input == NULL) return;

	// Initialize
	input->translation[0] = input->translation[1] = input->translation[2] = 0;
	dlg.Initialize(input->translation[0], input->translation[1], input->translation[2]);

	if(dlg.DoModal() == IDOK)
	{
		// Update input translation value
		input->translation[0] = dlg.m_fTx; input->translation[1] = dlg.m_fTy; input->translation[2] = dlg.m_fTz;

		//  Create Translation Matrix
		GzTrxMat(input->translation, trxMat);

		// Accumulate matrix
		for(int i=0;i<AAKERNEL_SIZE;i++)
			GzPushMatrix(m_pApplication->m_pRender[i], trxMat); 
	}
}

// Callback function for Scaling
void CCS580HWView::OnScale() 
{
	// TODO: Add your command handler code here
	CScaleDlg dlg;
	GzInput* input;
	GzMatrix scaleMat = 
	{ 
		1.0,	0.0,	0.0,	0.0, 
		0.0,	1.0,	0.0,	0.0, 
		0.0,	0.0,	1.0,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	};


	if(m_pApplication == NULL) return;

	input = m_pApplication->m_pUserInput;
	if(input == NULL) return;

	// Initialize
	input->scale[0] = input->scale[1] = input->scale[2] = 1;
	dlg.Initialize(input->scale[0], input->scale[1], input->scale[2]);

	if(dlg.DoModal() == IDOK)
	{
		// Update input scale value
		input->scale[0] = dlg.m_fSx; input->scale[1] = dlg.m_fSy; input->scale[2] = dlg.m_fSz;

		//  Create Scaling Matrix
		GzScaleMat(input->scale, scaleMat);

		// Accumulate matrix
		for(int i=0;i<AAKERNEL_SIZE;i++)
			GzPushMatrix(m_pApplication->m_pRender[i], scaleMat); 
	}
}


LRESULT CCS580HWView::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	bool up_pressed = NULL;
	bool down_pressed = NULL;
	bool left_pressed = NULL;
	bool right_pressed = NULL;
	bool one_pressed = NULL;
	bool two_pressed = NULL;
	bool three_pressed = NULL;
	bool four_pressed = NULL;
	float xita = 0;
	float phi = 0;
	switch (message)
     {
     case WM_KEYDOWN:
          switch (wParam)
          {
          case VK_LEFT:
			up_pressed = true;
			for(int i=0; i<AAKERNEL_SIZE; i++)
			{
				
				
				m_pApplication->m_pRender[i]->xita -= 10;
				//check if xita is out of boundary
				//if(m_pApplication->m_pRender[i]->xita <= 0)m_pApplication->m_pRender[i]->xita = 0;
				int phi_degree = m_pApplication->m_pRender[i]->phi;
				int xita_degree = m_pApplication->m_pRender[i]->xita;
				m_pApplication->m_pRender[i]->camera.position[0] = sin(xita_degree * PI/180) * cos(phi_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->camera.position[1] = sin(phi_degree * PI/180) * sin(xita_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->camera.position[2] = cos(xita_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->matlevel = 0;
				//float vector[4] = {0.0, 0.2, 0.0, 0.0};
				//Xform_view(m_pApplication->m_pRender[i]->camera.Xwi,vector);
				//m_pApplication->m_pRender[i]->camera.position[0] += vector[0];
				//m_pApplication->m_pRender[i]->camera.position[1] += vector[1];
				//m_pApplication->m_pRender[i]->camera.position[2] += vector[2];
				//m_pApplication->m_pRender[i]->matlevel = 0;
				GzBeginRender(m_pApplication->m_pRender[i]);
			}
			this->OnRender();

			break ;
			
          case VK_RIGHT:
            down_pressed = true;
			for(int i=0; i<AAKERNEL_SIZE; i++)
			{
				m_pApplication->m_pRender[i]->xita += 10;
				//check if xita is out of boundary
				//if(m_pApplication->m_pRender[i]->xita >= 180)m_pApplication->m_pRender[i]->xita = 180;
				int phi_degree = m_pApplication->m_pRender[i]->phi;
				int xita_degree = m_pApplication->m_pRender[i]->xita;
				m_pApplication->m_pRender[i]->camera.position[0] = sin(xita_degree * PI/180) * cos(phi_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->camera.position[1] = sin(phi_degree * PI/180) * sin(xita_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->camera.position[2] = cos(xita_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->matlevel = 0;
				//float vector[4] = {0.0, -0.2, 0.0, 0.0};
				//Xform_view(m_pApplication->m_pRender[i]->camera.Xwi,vector);
				//m_pApplication->m_pRender[i]->camera.position[0] += vector[0];
				//m_pApplication->m_pRender[i]->camera.position[1] += vector[1];
				//m_pApplication->m_pRender[i]->camera.position[2] += vector[2];
				//m_pApplication->m_pRender[i]->matlevel = 0;
				GzBeginRender(m_pApplication->m_pRender[i]);
			}
			this->OnRender();
            break ;
               
          case VK_UP:
			left_pressed = true;
			for(int i=0; i<AAKERNEL_SIZE; i++)
			{
				m_pApplication->m_pRender[i]->phi -= 10;
				int phi_degree = m_pApplication->m_pRender[i]->phi;
				int xita_degree = m_pApplication->m_pRender[i]->xita;
				m_pApplication->m_pRender[i]->camera.position[0] = sin(xita_degree * PI/180) * cos(phi_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->camera.position[1] = sin(phi_degree * PI/180) * sin(xita_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->camera.position[2] = cos(xita_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->matlevel = 0;
				//float vector[4] = {-0.2, 0.0, 0.0, 0.0};
				//Xform_view(m_pApplication->m_pRender[i]->camera.Xwi,vector);
				//m_pApplication->m_pRender[i]->camera.position[0] += vector[0];
				//m_pApplication->m_pRender[i]->camera.position[1] += vector[1];
				//m_pApplication->m_pRender[i]->camera.position[2] += vector[2];
				//m_pApplication->m_pRender[i]->matlevel = 0;
				GzBeginRender(m_pApplication->m_pRender[i]);
			}
			this->OnRender();
            break ;
               
          case VK_DOWN:
            right_pressed = true;
			for(int i=0; i<AAKERNEL_SIZE; i++)
			{
				m_pApplication->m_pRender[i]->phi += 10;
				int phi_degree = m_pApplication->m_pRender[i]->phi;
				int xita_degree = m_pApplication->m_pRender[i]->xita;
				m_pApplication->m_pRender[i]->camera.position[0] = sin(xita_degree * PI/180) * cos(phi_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->camera.position[1] = sin(phi_degree * PI/180) * sin(xita_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->camera.position[2] = cos(xita_degree * PI/180) * RADIUS;
				m_pApplication->m_pRender[i]->matlevel = 0;
				//float vector[4] = {0.2, 0.0, 0.0, 0.0};
				//Xform_view(m_pApplication->m_pRender[i]->camera.Xwi,vector);
				//m_pApplication->m_pRender[i]->camera.position[0] += vector[0];
				//m_pApplication->m_pRender[i]->camera.position[1] += vector[1];
				//m_pApplication->m_pRender[i]->camera.position[2] += vector[2];
				//m_pApplication->m_pRender[i]->matlevel = 0;
				GzBeginRender(m_pApplication->m_pRender[i]);
			}
			this->OnRender();
            break ;
		  case 0x31:
			one_pressed = true;
			
               break ;

          case 0x32:
            two_pressed = true;
			
               break ;
               
          case 0x33:
			three_pressed = true;
			
               break ;
               
          case 0x34:
            four_pressed = true;
			
               break ;
		  default:
			  break;
          }
	case WM_KEYUP:
          switch (wParam)
          {
          case VK_UP:
			up_pressed = false;
               break ;

          case VK_DOWN:
            down_pressed = false;
               break ;
               
          case VK_LEFT:
			left_pressed = false;
               break ;
               
          case VK_RIGHT:
            right_pressed = false;
               break ;

		  case 0x31:
			one_pressed = false;
               break ;

          case 0x32:
            two_pressed = false;
               break ;
               
          case 0x33:
			three_pressed = false;
               break ;
               
          case 0x34:
            four_pressed = false;
               break ;
		  default:
			  break;
          }

	 default:
		 break;
     }
	return CView::DefWindowProc(message, wParam, lParam);
}

int CCS580HWView::Xform_view(GzMatrix mat, GzCoord vec){
	float after[4];
	for(int i=0;i<4;i++){
		after[i] = mat[i][X]*vec[X] + 
					mat[i][Y]*vec[Y] +
					mat[i][Z]*vec[Z] +
					mat[i][3]*1;
	}

	if(after[3] != 0){
		vec[X] = after[X]/after[3];
		vec[Y] = after[Y]/after[3];
		vec[Z] = after[Z]/after[3];
	}else{
		vec[X] = after[X];
		vec[Y] = after[Y];
		vec[Z] = after[Z];
	}
	if(after[2]<0)
		return GZ_FAILURE;
	return GZ_SUCCESS;
}

