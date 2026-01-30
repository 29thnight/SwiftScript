# SwiftScript 문법 구현 목록

## 현재 구현 현황 요약

| 구분 | 상태 | 비고 |
|------|------|------|
| 렉서 (Lexer) | ✅ 완료 | 66개 토큰 |
| 파서 (Parser) | ✅ 대부분 완료 | 30+ AST 노드 |
| 타입 체커 | ✅ 기본 구현 | 정적 타입 검사 |
| 런타임 (VM) | 🟡 부분 완료 | 일부 기능 미지원 |

---

## 우선순위 1: 즉시 구현 필요

### 1.1 Struct 프로퍼티 직접 할당
- **현황**: 파싱 완료, 런타임 미지원
- **문제**: `point.x = 10` 형태의 할당이 동작하지 않음
- **필요 작업**:
  - [ ] `SET_PROPERTY` OpCode 구현
  - [ ] 구조체 인스턴스 프로퍼티 수정 로직
  - [ ] 값 의미론 유지하면서 프로퍼티 업데이트

### 1.2 Struct init에서 self 할당
- **현황**: 파싱 완료, 컴파일 미지원
- **문제**: `self.x = value` 형태가 init 내부에서 동작하지 않음
- **필요 작업**:
  - [ ] init 컨텍스트에서 self 바인딩
  - [ ] self.property 할당 컴파일 지원

### 1.3 Named Parameters 완전 구현
- **현황**: 파싱 완료, 런타임 매칭 불완전
- **문제**: `func greet(name: String)` 호출 시 `greet(name: "Kim")` 매칭 필요
- **필요 작업**:
  - [ ] 함수 호출 시 파라미터 이름 매칭
  - [ ] 외부 파라미터 이름과 내부 이름 구분 (`func greet(_ name: String)`)
  - [ ] 기본값 있는 파라미터 처리

### 1.4 Associated Values 런타임
- **현황**: AST 파싱 완료, 런타임 미지원
- **문제**: `case success(Int)` 형태의 enum 케이스 사용 불가
- **필요 작업**:
  - [ ] Associated value 저장 구조
  - [ ] 패턴 매칭에서 값 추출 (`case .success(let value)`)
  - [ ] switch 문에서 바인딩 지원

---

## 우선순위 2: 타입 안전성 강화

### 2.1 비트 연산자 구현
- **현황**: 토큰만 정의됨
- **필요 작업**:
  - [ ] `&` (AND) OpCode 및 VM 구현
  - [ ] `|` (OR) OpCode 및 VM 구현
  - [ ] `^` (XOR) OpCode 및 VM 구현
  - [ ] `~` (NOT) OpCode 및 VM 구현
  - [ ] `<<` (Left Shift) OpCode 및 VM 구현
  - [ ] `>>` (Right Shift) OpCode 및 VM 구현
  - [ ] 복합 할당: `&=`, `|=`, `^=`, `<<=`, `>>=`

### 2.2 프로토콜 준수 검증
- **현황**: 프로토콜 선언/채택 파싱 완료, 검증 미완
- **필요 작업**:
  - [ ] 컴파일 타임에 필수 메서드 구현 확인
  - [ ] 필수 프로퍼티 구현 확인
  - [ ] 에러 메시지 개선

### 2.3 Access Control 검증
- **현황**: 키워드 파싱 완료, 런타임 검증 없음
- **필요 작업**:
  - [ ] `private` 멤버 외부 접근 차단
  - [ ] `fileprivate` 파일 범위 제한
  - [ ] `internal` 모듈 범위 제한
  - [ ] `public` 공개 접근 허용

### 2.4 mutating 메서드 완전 구현
- **현황**: 파싱 완료, self 수정 불완전
- **필요 작업**:
  - [ ] mutating 메서드 내 self 수정 허용
  - [ ] 비-mutating 메서드에서 self 수정 차단
  - [ ] let 상수 구조체의 mutating 메서드 호출 차단

---

## 우선순위 3: 고급 기능

### 3.1 Property Observers
- **현황**: `willSet`, `didSet` 파싱 완료, 런타임 미지원
- **필요 작업**:
  - [ ] 프로퍼티 변경 전 `willSet` 호출
  - [ ] 프로퍼티 변경 후 `didSet` 호출
  - [ ] `newValue`, `oldValue` 암시적 파라미터

### 3.2 Lazy Properties
- **현황**: `lazy` 키워드 파싱 완료, 지연 초기화 미지원
- **필요 작업**:
  - [ ] 첫 접근 시점에 초기화
  - [ ] 초기화 상태 추적
  - [ ] 스레드 안전성 고려 (선택적)

### 3.3 Subscript 정의
- **현황**: 배열/딕셔너리 첨자 접근만 지원
- **필요 작업**:
  - [ ] 커스텀 subscript 선언 파싱
  - [ ] subscript get/set 구현
  - [ ] 다중 파라미터 subscript

### 3.4 where 절 런타임
- **현황**: for-in에서 파싱 완료, 필터링 미지원
- **필요 작업**:
  - [ ] `for item in array where condition` 필터링
  - [ ] switch case의 where 절

---

## 우선순위 4: 확장 기능

### 4.1 제네릭 (Generics)
- **현황**: 미구현
- **필요 작업**:
  - [ ] 제네릭 타입 파라미터 파싱 (`<T>`)
  - [ ] 제네릭 함수 (`func swap<T>`)
  - [ ] 제네릭 타입 (`struct Stack<Element>`)
  - [ ] 타입 제약 (`<T: Comparable>`)
  - [ ] where 절 제약

### 4.2 연산자 오버로딩
- **현황**: 미구현
- **필요 작업**:
  - [ ] 연산자 함수 선언 (`static func +`)
  - [ ] 커스텀 연산자 정의 (`prefix`, `infix`, `postfix`)
  - [ ] 우선순위 그룹

### 4.3 async/await
- **현황**: 미구현
- **필요 작업**:
  - [ ] `async` 함수 선언
  - [ ] `await` 표현식
  - [ ] Task 기본 지원
  - [ ] 비동기 런타임

### 4.4 Pattern Matching 확장
- **현황**: 기본 패턴만 지원
- **필요 작업**:
  - [ ] 튜플 패턴 (`let (x, y) = point`)
  - [ ] 옵셔널 패턴 (`case let x?`)
  - [ ] 타입 캐스팅 패턴 (`case let x as Int`)
  - [ ] 표현식 패턴 (`case 1...10`)

---

## 우선순위 5: 추가 타입 시스템

### 5.1 튜플 (Tuple)
- **현황**: 미구현
- **필요 작업**:
  - [ ] 튜플 리터럴 파싱 (`(1, "hello")`)
  - [ ] 이름있는 튜플 (`(x: 1, y: 2)`)
  - [ ] 튜플 분해 (`let (a, b) = tuple`)
  - [ ] 함수 다중 반환값

### 5.2 typealias
- **현황**: 미구현
- **필요 작업**:
  - [ ] `typealias` 선언 파싱
  - [ ] 타입 별칭 해석

### 5.3 Nested Types
- **현황**: 미구현
- **필요 작업**:
  - [ ] 타입 내부 타입 선언
  - [ ] 중첩 타입 접근 (`Outer.Inner`)

### 5.4 Optional Chaining 확장
- **현황**: 기본 구현 완료
- **필요 작업**:
  - [ ] 메서드 호출 체이닝 (`obj?.method()?.property`)
  - [ ] 첨자 접근 체이닝 (`array?[0]?.name`)

---

## 미구현 기능 (낮은 우선순위)

| 기능 | 설명 | 복잡도 |
|------|------|--------|
| `@` 속성 | `@discardableResult`, `@escaping` 등 | 중 |
| `#if` 조건부 컴파일 | 플랫폼별 코드 분기 | 중 |
| `defer` | 스코프 종료 시 실행 | 중 |
| `inout` 파라미터 | 참조 전달 | 중 |
| 가변 파라미터 | `func sum(_ numbers: Int...)` | 낮음 |
| `rethrows` | 에러 전파 | 낮음 |
| `@autoclosure` | 자동 클로저 | 낮음 |
| `Any`, `AnyObject` | 범용 타입 | 중 |
| `Codable` | 직렬화/역직렬화 | 높음 |
| `Equatable`, `Hashable` | 자동 합성 | 중 |

---

## 구현 완료된 기능

### 렉서 (66개 토큰)
- [x] 리터럴: Integer, Float, String, True, False, Null
- [x] 키워드: 49개 (func, class, struct, enum, protocol, extension 등)
- [x] 연산자: 39개 (산술, 비교, 논리, 비트, 할당, 범위)
- [x] 구분자: 9개 (괄호, 중괄호, 대괄호, 콤마 등)

### 파서/AST (30+ 노드)
- [x] 표현식: 리터럴, 식별자, 단항, 이항, 할당, 호출, 멤버 접근
- [x] 옵셔널: 강제 언래핑, 옵셔널 체이닝, nil 병합
- [x] 컬렉션: 배열 리터럴, 딕셔너리 리터럴, 첨자 접근
- [x] 제어문: if/else, guard, while, repeat-while, for-in, switch
- [x] 선언: var/let, func, class, struct, enum, protocol, extension
- [x] 에러: try/catch, throw, do-catch
- [x] 클로저: 파라미터, 반환 타입, 본문

### 타입 체커
- [x] 내장 타입 인식 (Int, Float, String, Bool, Array, Dictionary)
- [x] 함수 서명 검증
- [x] 옵셔널 타입 처리
- [x] 스코프 기반 심볼 테이블

### 런타임 (VM)
- [x] 47개 OpCode
- [x] 클래스 인스턴스 생성 및 메서드 호출
- [x] 구조체 값 의미론
- [x] 상속 및 super 호출
- [x] 클로저 캡처
- [x] 옵셔널 처리

---

## 테스트 현황

- `test_basic.cpp` - 기본 기능
- `test_class.cpp` - 클래스 (11개 테스트)
- `test_struct.cpp` - 구조체 (10개 테스트)
- `test_enum.cpp` - 열거형
- `test_protocol.cpp` - 프로토콜
- `test_extension.cpp` - 확장
- `test_optional.cpp` - 옵셔널
- `test_closure.cpp` - 클로저 (9개 테스트)
- `test_switch.cpp` - switch 문 (4개 테스트)

---

*마지막 업데이트: 2026-01-30*
