# Tetris
테트리스

## 게임 흐름
1. 시작 시 플레이
2. 점수에 따라 레벨 및 속도 변경
3. 블록 생성 실패 시 게임 종료

## 구현

### UI
CPen 및 CBrush RAII로 생성 삭제 구현
IDrawable 객체는 CRenderer에서 보유 후 WM_PAINT마다 전체 OnDraw 호출을 통해 그리기
IDrawable은 Factory 함수로만 생성할 수 있도록 적용 → 무조건 CRenderer 배열에 저장된다

### 게임 로직
블록은 최대 가로 세로 4만큼 길이 이므로, 전체 블록 별 회전 값을 [8][4][4] 배열에 적용
타입 회전 별로 4개의 포인트를 가져올 수 있다.

충돌은 현재 해당 4개의 포인트값과 비교

틱은 QueryPerformance 및 QueryCounter를 통해 구현.

1000점 당 1레벨, 레벨 당 드랍 시간 2 → 0.1f까지 조정

블록 생성 시도 시 (SetNextBlock) 해당 위치가 막혀있으면 게임 패배

