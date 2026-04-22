# watcom-game-dev

Watcom C/C++ 10.6으로 만드는 DOS 실모드 게임 엔진. VGA Mode 13h (320x200, 256색) 타겟. macOS에서 개발하고 DOSBox 안에서 컴파일합니다.

## 엔진 모듈

- **GFX** — 더블 버퍼링 VGA 렌더링, vsync, 프리미티브 드로잉, 컴파일드 스프라이트 실행, 팔레트
- **SPRITE** — 바이너리 `.SPR` 로더. 스프라이트 프레임은 선형 버퍼를 타겟으로 하는 x86 기계어(`MOV`) 시퀀스
- **FONT** — 한글(16x16 조합) + 영문(8x16) 비트맵 렌더러
- **IMG** — raw indexed `.IMG` 로더
- **SFX** — Sound Blaster PCM 소프트웨어 믹서 (오토-이닛 DMA + IRQ)
- **SOUND** — OPL2 (AdLib) VGM 플레이어. 타이머 ISR 체인에 후킹
- **TIMER** — PIT를 1000Hz로 재프로그래밍, 밀리초 클록 제공
- **INPUT** — INT 9 ISR, 실시간 키 상태 배열 + 이벤트 링 버퍼
- **SCRNCAP** — 화면 캡쳐

## 필요 환경

- [DOSBox](https://www.dosbox.com/) (0.74 이상)
- [Bun](https://bun.sh/) — 에셋 파이프라인용
- macOS — 빌드 스크립트가 `/Applications/dosbox.app` 경로를 사용합니다

## 빌드 & 실행

```bash
./build.sh          # 에셋 변환 + DOSBox에서 컴파일 → SRC/GAME.EXE
./run.sh            # DOSBox에서 GAME.EXE 실행
./run.sh OTHER.EXE  # 다른 실행 파일 실행
./convert.sh        # 에셋 파이프라인만 실행 (build.sh에서 자동 호출)
```

`SRC/GAME.CPP`는 모든 서브시스템을 초기화/종료하는 최소 엔진 스모크 테스트입니다. 게임을 작성할 때 교체하세요.

## 프로젝트 구조

```
SRC/               Watcom C++ 엔진 소스 (8.3 대문자 파일명)
  GAME.CPP         엔진 스모크 테스트 (교체 대상)
  GFX.CPP/H        VGA Mode 13h
  INPUT.CPP/H      INT 9 키보드 핸들러
  TIMER.CPP/H      PIT 1000Hz 타이머
  SOUND.CPP/H      OPL2 VGM 플레이어
  SFX.CPP/H        Sound Blaster PCM 믹서
  SPRITE.CPP/H     .SPR 로더
  FONT.CPP/H       폰트 렌더러
  IMG.CPP/H        .IMG 로더
  SCRNCAP.CPP/H    화면 캡쳐
  PALETTE.H        생성된 256색 팔레트
tools/             에셋 파이프라인 (TypeScript/Bun)
SFXTEST/           SFX 엔진 테스트 하니스
WATCOM/            Watcom C/C++ 10.6 툴체인
```

## 라이센스

[MIT](LICENSE)
