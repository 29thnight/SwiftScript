<p align="center">
  <img src="https://raw.githubusercontent.com/29thnight/Swive/master/Swive.png" alt="Swive" height="190" />
</p>

<h1 align="center">Swive</h1>

<p align="center">
  <strong>Swift-inspired scripting language with LSP support for VS Code</strong>
</p>

---

Swive는 Swift 문법의 핵심 요소를 지원하는 학습/실험용 스크립트 언어입니다.  
이 확장은 **구문 강조**, **LSP 진단(에러/경고)**, **시맨틱 토큰**, **프로젝트 빌드 & 실행**을 VS Code에서 제공합니다.

## ✨ 기능

- 🎨 **구문 강조** — `.ss` 파일에 대한 TextMate 문법 및 시맨틱 토큰 지원
- 🔍 **LSP 진단** — 실시간 에러/경고 표시 (SwiveServer 기반)
- ▶️ **빌드 & 실행** — 에디터 타이틀바에서 한 클릭으로 `swive exec` 실행
- 🛑 **정지** — 실행 중인 프로세스를 터미널에서 중단
- 📁 **프로젝트 생성** — `Swive: Add Project` 명령으로 프로젝트 템플릿 자동 생성

## 📦 지원 파일

| 확장자 | 설명 |
|--------|------|
| `.ss` | Swive 스크립트 파일 |
| `.ssproject` | Swive 프로젝트 파일 (XML) |

## 🚀 시작하기

### 1. 프로젝트 생성

커맨드 팔레트(`Ctrl+Shift+P`)에서 **Swive: Add Project** 실행

프로젝트 이름을 입력하고 폴더를 선택하면 다음 구조가 자동 생성됩니다:

```
MyProject/
├── MyProject.ssproject
├── Scripts/
│   └── main.ss
├── Libs/
└── bin/
    └── Debug/
```

### 2. 코드 작성

`Scripts/main.ss`:

```swift
print("Hello, World!")
```

### 3. 빌드 & 실행

- 에디터 우측 상단의 ▶️ 버튼 클릭
- 또는 커맨드 팔레트에서 **Swive: Compile & Run** 실행

## ⚙️ 설정

| 설정 | 기본값 | 설명 |
|------|--------|------|
| `swive.swivePath` | `""` (번들 사용) | swive.exe (Unified CLI) 경로 |
| `swive.serverPath` | `""` (번들 사용) | SwiveServer.exe (LSP) 경로 |
| `swive.buildType` | `Debug` | 빌드 구성 (`Debug` / `Release`) |
| `swive.trace.server` | `off` | LSP 클라이언트 트레이스 레벨 |

## 📋 명령어

| 명령어 | 설명 |
|--------|------|
| `Swive: Compile & Run` | 프로젝트를 컴파일하고 실행 |
| `Swive: Stop` | 실행 중인 프로세스 중단 |
| `Swive: Add Project` | 새 Swive 프로젝트 생성 |

## 🗂️ 프로젝트 파일 (.ssproject)

```xml
<Project>
    <Name>MyProject</Name>
    <Entry>Scripts/main.ss</Entry>
    <ImportRoots>
        <Root>Libs</Root>
        <Root>Scripts</Root>
    </ImportRoots>
</Project>
```

## 📖 Swive Language Reference

### 기본 문법

```swift
// 변수와 상수
var count = 0
let name: String = "Swive"

// 출력
print("Hello, ${name}!")

// 함수
func add(a: Int, b: Int) -> Int {
    return a + b
}
```

### 타입 시스템

| 타입 | 설명 | 예시 |
|------|------|------|
| `Int` | 64비트 정수 | `42`, `-7` |
| `Float` | 64비트 부동소수점 | `3.14` |
| `Bool` | 불리언 | `true`, `false` |
| `String` | 문자열 | `"hello"` |
| `Any` | 모든 타입 수용 | - |
| `Array<T>` | 동적 배열 | `[1, 2, 3]` |
| `Dictionary<K,V>` | 키-값 맵 | `["key": value]` |

### 클래스 & 구조체

```swift
class Animal {
    var name: String
    var age: Int

    init(name: String, age: Int) {
        self.name = name
        self.age = age
    }

    func speak() -> String {
        return "${name} says hello"
    }
}

struct Point {
    var x: Float
    var y: Float

    mutating func translate(dx: Float, dy: Float) {
        x += dx
        y += dy
    }
}
```

### 열거형 & 패턴 매칭

```swift
enum Result {
    case success(message: String)
    case failure(code: Int, message: String)
}

switch result {
    case .success(message):
        print("성공: ${message}")
    case .failure(code, message):
        print("실패 ${code}: ${message}")
}
```

### 프로토콜 & 제네릭

```swift
protocol Describable {
    func describe() -> String
}

func findMax<T>(array: Array<T>) -> T where T: Comparable {
    var max = array[0]
    for item in array {
        if item > max { max = item }
    }
    return max
}
```

### 옵셔널

```swift
var name: String? = "Kim"

if let n = name {
    print("이름: ${n}")
}

let displayName = name ?? "Unknown"
```

### 에러 핸들링 (expected)

```swift
func divide(a: Int, b: Int) -> Int expected String {
    if (b == 0) {
        return expected.error("Division by zero")
    }
    return a / b
}

let result = divide(a: 10, b: 0)
switch result {
    case .value(v): print("결과: ${v}")
    case .error(e): print("에러: ${e}")
}
```

### 익스텐션

```swift
extension Int {
    func isEven() -> Bool {
        return self % 2 == 0
    }
}
```

### 어트리뷰트

```swift
[Deprecated("Use NewClass instead")]
class OldClass { }

class Player {
    [Range(0, 100)]
    public var health: Int
}
```

### 네이티브 바인딩

```swift
[Native.Class("Vector3")]
class Vector3 {
    [Native.Field("x")]
    var x: Float

    [Native.InternalCall]
    func magnitude() -> Float;
}
```

### 키워드 (49개)

**선언:** `func` `class` `struct` `enum` `protocol` `extension` `attribute` `var` `let` `init` `deinit` `static` `override` `mutating` `import`

**제어 흐름:** `if` `else` `guard` `switch` `case` `default` `for` `in` `while` `repeat` `break` `continue` `return`

**접근 제어:** `public` `private` `internal` `fileprivate`

**기타:** `expected` `get` `set` `willSet` `didSet` `lazy` `weak` `unowned` `as` `is` `where` `true` `false` `nil` `null` `self` `super`

---

## Swive CLI

Swive는 통합 CLI를 제공합니다:

```
swive build MyProject.ssproject              # 컴파일만
swive build MyProject.ssproject -c Release   # Release 빌드
swive run bin/Debug/MyProject.ssasm          # 바이트코드 실행
swive exec MyProject.ssproject               # 컴파일 + 실행
swive exec MyProject.ssproject -c Debug --stats
```

---

## 📄 라이선스

© ideneb
