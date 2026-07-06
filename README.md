# Tetris
테트리스

## 게임 흐름
1. 시작 시 플레이
2. 점수에 따라 레벨 및 속도 변경
3. 블록 생성 실패 시 게임 종료

## 구현

### UI
IDrawable: 그리는 객체라면 해당 클래스 상속, Draw::CreateDrawable로만 생성하여 CRenderer의 그리는 대상 배열에 저장

CPen, CBrush: 그리기 용 Pen 및 Brush RAII

CRenderer: HDC, Hwnd 등 제공 용 및 전체적인 그리기 관리

### 게임 로직
CTetrisManager: 전체적인 테트리스 관련 로직 처리

CBoard: 테트리스가 일어나는 보드 UI. 프리뷰 박스, 점수, 게임 오버 등 표기

CTetromino: 블록 UI, 타입 별 회전에 대한 전체 블록 값을 [8][4][4] 배열로 보관

틱은 QueryPerformance 및 QueryCounter를 통해 구현.

### 점수 계산
1000점 당 1레벨, 레벨 당 드랍 시간 2 → 0.1f까지 조정
