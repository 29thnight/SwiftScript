import MathLib
import TestModule

// 제네릭 구조체
struct Pair<T, U> {
    var first: T
    var second: U

    init(first: T, second: U) {
        self.first = first
        self.second = second
    }

}

extension Animal {
    func makeSound() -> String {
        return "Some sound"
    }
}

// 열거형 with 연관값
enum NetworkResult {
    case success(data: String)
    case failure(code: Int, message: String)
}

func divide(a: Int, b: Int) -> Int expected String {
    if (b == 0) {
        return expected.error("Division by zero")
    }
    return a / b
}

// 메인 함수
func main() {
    // 변수
    let pair = Pair<Int, String>(first: 42, second: "hello")
    print("Pair: ${pair.first}, ${pair.second}")

    // 옵셔널
    let input: String? = readLine()
    let name = input ?? "Anonymous"
    print("Hello, ${name}!")

    // // 에러 핸들링
    // // switch 패턴 매칭
    let result = divide(a: 10, b: 0)
    switch result {
        case .value(v): print("Result: ${v}")
        case .error(e): print("Error: ${e}")
    }

    let animal = Animal(name: "Buddy", age: 30)
    print("Animal: ${animal.toString()}")
    print("Sound: ${animal.makeSound()}")

    // // if-let 스타일
    if let v = divide(a: 10, b: 2) {
        print("Success: ${v}")
    } else {
        print("Failed")
    }

    // // 패턴 매칭
    let result_val = NetworkResult.success(data: "OK")
    switch result_val {
        case .success(data):
            print("Data: ${data}")
        case .failure(code, message):
            print("Error ${code}: ${message}")
    }

    // //for-in with where
    for i in 1...100 where i % 15 == 0 {
        print("FizzBuzz: ${i}")
    }

}