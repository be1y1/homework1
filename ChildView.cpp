// ChildView.cpp: CChildView 클래스의 구현


#include "pch.h"
#include "framework.h"
#include "homework1.h"
#include "ChildView.h"
#include "resource.h"

#include <queue>
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
    m_selEdgeStart = -1;
    m_selPathStart = -1;
    m_bmpLoaded = FALSE;
}

CChildView::~CChildView()
{
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_CREATE()
END_MESSAGE_MAP()


// 윈도우 스타일 설정
BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        ::LoadCursor(nullptr, IDC_ARROW),
        reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
        nullptr);

    return TRUE;
}

// 비트맵 로드
int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_bmpLoaded = m_bitmap.LoadBitmap(IDB_CAMPUS); // 캠퍼스 사진(BITMAP) 리소스
    return 0;
}

//      그래프 함수들

// 클릭 근처에 점이 있는지 찾기
int CChildView::HitTestNode(CPoint pt, int radius)
{
    int r2 = radius * radius;
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        int dx = pt.x - m_nodes[i].pt.x;
        int dy = pt.y - m_nodes[i].pt.y;
        if (dx * dx + dy * dy <= r2)
            return i;
    }
    return -1;
}

void CChildView::AddNode(CPoint pt)
{
    Node n;
    n.pt = pt;
    m_nodes.push_back(n);
}

void CChildView::AddEdge(int u, int v)
{
    if (u == v) return;

    for (const auto& e : m_edges)
    {
        if ((e.u == u && e.v == v) || (e.u == v && e.v == u))
            return;
    }

    Edge e;
    e.u = u;
    e.v = v;

    int dx = m_nodes[u].pt.x - m_nodes[v].pt.x;
    int dy = m_nodes[u].pt.y - m_nodes[v].pt.y;
    e.length = std::sqrt(double(dx * dx + dy * dy));

    m_edges.push_back(e);
}

// 다익스트라 알고리즘 
void CChildView::ComputeShortestPath(int start, int goal)
{
    const int N = (int)m_nodes.size();
    if (start < 0 || start >= N || goal < 0 || goal >= N)
        return;

    // 인접 리스트 구성
    std::vector<std::vector<std::pair<int, double>>> adj(N);
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        const Edge& e = m_edges[i];
        adj[e.u].push_back(std::make_pair(e.v, e.length));
        adj[e.v].push_back(std::make_pair(e.u, e.length)); // 무방향 그래프
    }

    const double INF = 1e100;
    std::vector<double> dist(N, INF);
    std::vector<int>    prev(N, -1);

    typedef std::pair<double, int> NodePair;

    std::priority_queue<
        NodePair,
        std::vector<NodePair>,
        std::greater<NodePair>
    > pq;

    dist[start] = 0.0;
    pq.push(NodePair(0.0, start));

    while (!pq.empty())
    {
        NodePair top = pq.top();
        pq.pop();
        double cdist = top.first;
        int    u = top.second;

        if (cdist > dist[u])
            continue;
        if (u == goal)
            break;

        // 인접 정점들 탐색
        for (size_t i = 0; i < adj[u].size(); ++i)
        {
            int    v = adj[u][i].first;
            double w = adj[u][i].second;

            double nd = cdist + w;
            if (nd < dist[v])
            {
                dist[v] = nd;
                prev[v] = u;
                pq.push(NodePair(nd, v));
            }
        }
    }

    // 경로 복원
    m_shortestPath.clear();
    if (dist[goal] == INF)
        return; // 경로 없음

    int cur = goal;
    while (cur != -1)
    {
        m_shortestPath.push_back(cur);
        cur = prev[cur];
    }

    std::reverse(m_shortestPath.begin(), m_shortestPath.end());
}

//      그리기 코드

void CChildView::DrawBackground(CDC& dc)
{
    if (!m_bmpLoaded) return;

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);

    BITMAP bm;
    m_bitmap.GetBitmap(&bm);
    CBitmap* pOld = memDC.SelectObject(&m_bitmap);

    CRect rc;
    GetClientRect(&rc);

    dc.StretchBlt(
        0, 0, rc.Width(), rc.Height(),
        &memDC,
        0, 0, bm.bmWidth, bm.bmHeight,
        SRCCOPY
    );

    memDC.SelectObject(pOld);
}


void CChildView::DrawGraph(CDC& dc)
{
    // 선분
    CPen penEdge(PS_SOLID, 1, RGB(70, 70, 70));
    CPen* pOldPen = dc.SelectObject(&penEdge);

    for (const auto& e : m_edges)
    {
        CPoint p1 = m_nodes[e.u].pt;
        CPoint p2 = m_nodes[e.v].pt;
        dc.MoveTo(p1);
        dc.LineTo(p2);

        CString s;
        s.Format(L"%.1f", e.length);

        int mx = (p1.x + p2.x) / 2;
        int my = (p1.y + p2.y) / 2;
        dc.TextOut(mx + 5, my + 5, s);
    }

    dc.SelectObject(pOldPen);

    // 점
    CBrush brushNode(RGB(0, 0, 255));
    CBrush* pOldBrush = dc.SelectObject(&brushNode);

    const int R = 4;
    for (const auto& n : m_nodes)
    {
        dc.Ellipse(n.pt.x - R, n.pt.y - R,
            n.pt.x + R, n.pt.y + R);
    }

    dc.SelectObject(pOldBrush);
}


// 빨간색 최단경로 표시
void CChildView::DrawShortestPath(CDC& dc)
{
    if (m_shortestPath.size() < 2) return;

    CPen penPath(PS_SOLID, 3, RGB(255, 0, 0));
    CPen* pOld = dc.SelectObject(&penPath);

    for (size_t i = 1; i < m_shortestPath.size(); ++i)
    {
        CPoint p1 = m_nodes[m_shortestPath[i - 1]].pt;
        CPoint p2 = m_nodes[m_shortestPath[i]].pt;

        dc.MoveTo(p1);
        dc.LineTo(p2);
    }

    dc.SelectObject(pOld);
}


// 전체 화면 그리기
void CChildView::OnPaint()
{
    CPaintDC dc(this);

    DrawBackground(dc);
    DrawGraph(dc);
    DrawShortestPath(dc);
}

//      마우스 입력

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
    bool bCtrl = (nFlags & MK_CONTROL) != 0;
    bool bShift = (nFlags & MK_SHIFT) != 0;
    bool bAlt = (::GetKeyState(VK_MENU) & 0x8000) != 0;

    int hit = HitTestNode(point);

    // Ctrl + 클릭 → 점 추가
    if (bCtrl && !bAlt && !bShift)
    {
        if (hit == -1)
        {
            AddNode(point);
        }
        Invalidate(FALSE);
    }
    // Alt + 클릭 → 두 점 선택하여 선분 생성
    else if (bAlt && !bCtrl && !bShift)
    {
        if (hit == -1) return;

        if (m_selEdgeStart == -1)
        {
            m_selEdgeStart = hit;
        }
        else
        {
            AddEdge(m_selEdgeStart, hit);
            m_selEdgeStart = -1;
            Invalidate(FALSE);
        }
    }
    // Shift + 클릭 → 최단경로 계산
    else if (bShift && !bCtrl && !bAlt)
    {
        if (hit == -1) return;

        if (m_selPathStart == -1)
        {
            m_selPathStart = hit;
            m_shortestPath.clear();
        }
        else
        {
            ComputeShortestPath(m_selPathStart, hit);
            m_selPathStart = -1;
            Invalidate(FALSE);
        }
    }

    CWnd::OnLButtonDown(nFlags, point);
}
