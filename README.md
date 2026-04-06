# watcom-game-dev

Watcom C/C++ 10.6으로 만드는 DOS 종스크롤 슈팅 게임. VGA Mode 13h (320x200, 256색) 타겟. macOS에서 개발하고 DOSBox 안에서 컴파일합니다.

## 주요 특징

- **더블 버퍼링 VGA 렌더링** — vsync 동기화
- **컴파일드 스프라이트** — 스프라이트 프레임이 x86 기계어(MOV 명령)로 구성, 픽셀 루프 없음
- **컴파일드 폰트 시스템** — 한글(8x4x4벌 16x16) + 영문(8x16) 비트맵 폰트를 빌드 시 x86 코드로 컴파일, 미리 컴파일되지 않은 글자는 소프트웨어 폴백으로 출력
- **OPL2 VGM 플레이어** — AdLib FM 합성을 통한 VGM 음악 재생
- **1000Hz 타이머** — PIT를 재프로그래밍하여 밀리초 정밀도 델타타임 게임 루프 구현
- **커스텀 키보드 핸들러** — INT 9 ISR, 실시간 키 상태 배열 + 이벤트 버퍼

## 필요 환경

- [DOSBox](https://www.dosbox.com/) (0.74 이상)
- [Bun](https://bun.sh/) (에셋 파이프라인용)
- macOS (빌드 스크립트가 `/Applications/dosbox.app` 경로 사용)

## 빌드 & 실행

```bash
./build.sh      # 에셋 변환 + DOSBox에서 컴파일 → SRC/GAME.EXE
./run.sh        # DOSBox에서 GAME.EXE 실행
```

## 프로젝트 구조

```
SRC/            Watcom C++ 소스 코드 (8.3 대문자 파일명)
  GAME.CPP      메인 루프, 게임 상태, 엔티티 관리
  GFX.CPP/H     VGA Mode 13h 그래픽, 컴파일드 스프라이트 렌더러
  INPUT.CPP/H   INT 9 키보드 핸들러
  TIMER.CPP/H   PIT 1000Hz 타이머 (INT 8)
  SOUND.CPP/H   OPL2 VGM 음악 플레이어
  SPRITE.CPP/H  바이너리 .SPR 스프라이트 로더
  FONT.CPP/H    한글/영문 컴파일드 폰트 렌더러

tools/          에셋 파이프라인 (TypeScript/Bun)
  mkpalette.ts  256색 팔레트 생성기
  mksprite.ts   PNG → 컴파일드 스프라이트 변환기
  mkfont.ts     비트맵 폰트 → x86 글리프 컴파일러

fonts/          비트맵 폰트 파일 (여러 서체, 교체 가능)
assets/         원본 PNG 스프라이트 시트
VGM/            VGM 음악 파일
WATCOM/         Watcom C/C++ 10.6 툴체인 (헤더 + 라이브러리)
```

## 폰트 시스템

UTF-8 문자열로 한글/영문 혼합 텍스트 출력을 지원합니다:

```c
font_puts(x, y, "Hello 한글!", color);
```

- **빠른 경로**: 소스 코드에서 사용된 문자를 빌드 시 x86으로 미리 컴파일
- **폴백**: 미등록 문자는 런타임에서 8x4x4벌 합성 알고리즘으로 출력

폰트 교체는 `--eng`, `--han` 플래그로 간단히 가능합니다:

```bash
bun tools/mkfont.ts --eng fonts/ENG_MAX.FNT --han fonts/HAN_DEW_MYUNG.FNT
```

## 라이센스

[MIT](LICENSE)
