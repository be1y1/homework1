// ChildView.h: CChildView 클래스의 인터페이스

#pragma once

#include <vector>   // std::vector 사용

// CChildView 창

class CChildView : public CWnd
{
    // 생성입니다.
public:
    CChildView();

    // 특성입니다.
public:

    // 작업입니다.
public:

    // 재정의입니다.
protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    // 구현입니다.
public:
    virtual ~CChildView();

protected:
    // ===== 그래프용 구조체 =====
    struct Node {
        CPoint pt;      // 점 위치 (화면 좌표)
    };

    struct Edge {
        int    u;       // 시작 노드 인덱스
        int    v;       // 끝 노드 인덱스
        double length;  // 거리(픽셀)
    };

    // ===== 데이터 멤버 =====
    std::vector<Node> m_nodes;        // 점 목록
    std::vector<Edge> m_edges;        // 간선 목록

    int m_selEdgeStart;               // Alt로 선분 만들 때 첫 번째 점
    int m_selPathStart;               // Shift로 최단경로 구할 때 시작 점
    std::vector<int> m_shortestPath;  // 최단 경로에 포함된 노드 인덱스들

    CBitmap m_bitmap;                 // 캠퍼스 사진 비트맵
    bool    m_bmpLoaded;              // 비트맵 로딩 여부

    // ===== 내부 함수들 =====
    int   HitTestNode(CPoint pt, int radius = 6);           // 클릭 위치 근처 노드 찾기
    void  AddNode(CPoint pt);                               // 새 점 추가
    void  AddEdge(int u, int v);                            // 간선 추가
    void  ComputeShortestPath(int start, int goal);         // 다익스트라

    void  DrawBackground(CDC& dc);                          // 배경(사진) 그리기
    void  DrawGraph(CDC& dc);                               // 점 + 선분 그리기
    void  DrawShortestPath(CDC& dc);                        // 최단경로(빨간 선) 그리기

    // 생성된 메시지 맵 함수
protected:
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()
};
