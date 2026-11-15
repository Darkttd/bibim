# Bibim

[English](README.md)

Bibim은 C++로 작성된 2D 게임 엔진으로, Halak Shin이 개발했습니다(2010–2014).
Windows(Direct3D 9)와 Android / iOS(OpenGL ES 2.0)를 대상으로 하며, 에셋 처리와 디버깅을 위한
C# 도구가 함께 제공됩니다.

## 플랫폼

| 플랫폼 | 렌더러 | 빌드 파일 | 상태 |
|---|---|---|---|
| Windows | Direct3D 9 | `build/vs2008` – `build/vs2019` (Visual Studio) | 주 플랫폼 |
| Android | OpenGL ES 2.0 | `build/eclipse` (Java 쪽만 있음, 네이티브 빌드 파일은 이 저장소에 없음) | 과거 지원 |
| iOS | OpenGL ES 2.0 | `build/xcode6` | 과거 지원 |
| Linux | OpenGL ES 2.0 | – | 포팅 진행 중 |

## 기능

| 영역 | 모듈 (`src/`) |
|---|---|
| 코어 | `Foundation`(수학, 문자열, 리플렉션), `Module`(서비스 트리), `Component`, `Time`(타임라인), `Threading`, `Diagnostics` |
| 그래픽 | `Graphics`(Direct3D 9 / OpenGL ES 2.0), `GFX`(파티클), `Typography`(FreeType 폰트), `Animation`(스프라이트, 이징) |
| UI | `UI`(위젯, 레이아웃, 이벤트, 효과) |
| 게임플레이 | `Gameplay`, `Collision2D` |
| 스크립팅 | lua_tinker를 통한 Lua 5.2, Bibim 자체 `Scripting` VM |
| 입출력 | `Asset`(로컬 파일, MPQ 아카이브, 네트워크 에셋 서버), `FileSystem`, `IO`, `Net` |
| 플랫폼 | `Framework`(게임 루프, 창), `Input`(키보드, 마우스, IME), `Audio` |

## 저장소 구조

| 경로 | 내용 |
|---|---|
| `include/Bibim/` | 공개 헤더 |
| `src/` | 엔진 소스 (모듈별 디렉터리) |
| `extlibs/` | 함께 포함된 서드파티 라이브러리 |
| `build/` | 툴체인별 IDE 프로젝트, `Doxyfile` |
| `lib/` | 빌드 결과물과, 게임 프로젝트가 가져다 쓰는 property sheet |
| `tools/` | C# 도구 (`Bibim.Toolkit`, `Bibim.Baaaaaab`) |
| `bin/` | 미리 빌드된 도구 |
| `docs/` | 메모 |

## Windows에서 빌드하기

필요한 것:

- Visual Studio (아래 설명은 VS2019 솔루션 기준)
- 구 DirectX SDK (June 2010) — 프로젝트가 설치 프로그램이 설정하는 `$(DXSDK_DIR)`을 참조합니다

순서:

1. 저장소 루트에서 `BIBIM_DIR.bat`을 한 번 실행합니다. 환경 변수 `BIBIM_DIR`을 이 디렉터리로
   설정합니다. 실행 후 Visual Studio를 다시 시작하세요.
2. `build/vs2019/Bibim.sln`을 열고 Win32 `Debug` 또는 `Release` 구성을 빌드합니다.
   엔진과 함께 포함된 라이브러리가 `lib/vs2019/Win32/<구성>/`에 생성됩니다.
3. 게임 프로젝트에서 property sheet `lib/vs2019/Win32/<구성>/Bibim.props`를 가져옵니다.
   엔진에 필요한 include 경로, 라이브러리 경로, 라이브러리가 추가됩니다.

`publish.py`는 이 property sheet와 단일 파일 소스 `src/Bibim.cpp`, `include/Bibim/All.h`를
다시 생성합니다. 소스 파일을 추가하거나 의존성을 바꾼 뒤에 실행하세요.

## 도구

미리 빌드된 도구는 `bin/`에, 소스는 `tools/`(`tools/Bibim.Toolkit.sln`)에 있습니다.

- **Bibim.Toolkit** — 에셋 파이프라인 라이브러리 (원본 에셋을 Bibim의 `.ab` 형식으로 변환)
- **Bibim.Baaaaaab** — 콘솔, UI 인스펙터, 에셋 서버를 갖춘 데스크톱 도구

Windows의 Debug 빌드는 각 에셋을 먼저 localhost의 에셋 서버에 요청하고, 실패하면 로컬 파일을 읽습니다.
원본 에셋 파일에서 바로 에셋을 받아 오려면 에셋 서버를 켜 두세요.

```
bin\Bibim.Baaaaaab.exe console /class:AssetServer
```

### 리눅스(Mono)에서 콘솔 프로그램 실행

`Bibim.Baaaaaab.Cli`는 `Bibim.Baaaaaab.exe console`과 같은 콘솔 프로그램을 WPF 없이 실행하므로, 리눅스에서도
Mono로 쓸 수 있습니다. 표준 입력의 각 줄은 실행 중인 프로그램에 명령으로 전달됩니다(예: `exit`). 표준 입력을
닫거나, 터미널에서 실행 중일 때 Ctrl+C를 누르면 창을 닫을 때와 같은 방식으로 종료합니다. Ctrl+C를 한 번 더 누르면
즉시 종료합니다. 표준 입력이 닫히면 종료하므로, 터미널 없이 서버를 띄우면(예: `< /dev/null`, `nohup`, systemd)
바로 종료됩니다. 이때는 `tail -f /dev/null | mono ...`처럼 표준 입력을 열어 두세요.

```
xbuild /p:Configuration=Release tools/Bibim.Baaaaaab.Cli/Bibim.Baaaaaab.Cli.csproj
cd bin
mono ../tools/Bibim.Baaaaaab.Cli/bin/x86/Release/Bibim.Baaaaaab.Cli.exe console /class:AssetServer
```

Windows 도구처럼 `bin/Plugin/*.dll`을 불러오도록 `bin/`에서 실행하세요. 일부 레시피(텍스처 쿠킹, Lua·HLSL
컴파일 등)는 아직 리눅스에서 동작하지 않습니다. [#27](https://github.com/Darkttd/bibim/issues/27)을 참고하세요.

## Bibim을 사용하는 프로젝트

- [Maple](https://github.com/Darkttd/Maple) — 피크로스 퍼즐 게임 *모에모에 네모로직(Moemoe Nemologic)*

## 소스 작성 규칙

- **플랫폼별 코드**는 접미사가 붙은 파일에 둡니다. 예: `GameFramework.Windows.cpp`,
  `GraphicsDevice.GLES2.cpp`, `Thread.Unix.cpp`. 각 파일은 `include/Bibim/Config.h`의
  `BIBIM_PLATFORM_*` / `BIBIM_USE_*` 매크로로 감싸져 있습니다.
- **줄바꿈과 BOM**: 각 파일의 기존 줄바꿈(대부분 CRLF)과 BOM을 그대로 유지합니다.
  파일 전체를 다시 포맷하지 마세요.
- **파일 끝 개행**: 모든 텍스트 파일은 개행으로 끝납니다(파일의 줄바꿈에 맞춰 CRLF 또는 LF). CI가
  `.github/scripts/check-final-newline.sh`로 검사하며, `extlibs/`, `tools/References/`, `bin/`은 제외합니다.
- **인코딩**: 한글 주석처럼 ASCII가 아닌 문자가 들어간 소스 파일은 반드시 **BOM이 있는 UTF-8**로
  저장합니다. Visual Studio 프로젝트가 `/utf-8` 옵션을 쓰지 않아서, MSVC는 BOM으로 UTF-8임을 판별합니다.

## 라이선스

Bibim은 원작자와 협의하여 MIT 라이선스로 지정하는 중입니다. `LICENSE` 파일을 추가하는 PR이
upstream에서 진행 중입니다: [halak/bibim#2](https://github.com/halak/bibim/pull/2)

`extlibs/`의 서드파티 라이브러리는 각자의 라이선스를 따릅니다. `LICENSE-BUNDLE`을 참고하세요.
특히 **irrKlang**(`extlibs/irrKlang-1.4.0`)은 오픈소스가 아니며, 바이너리는 비상업 제품에서만
재배포할 수 있습니다. 자세한 내용은 `extlibs/irrKlang-1.4.0/readme.txt`를 참고하세요.
