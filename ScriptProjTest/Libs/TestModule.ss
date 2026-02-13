// 프로토콜
protocol Printable {
    func toString() -> String
}

// 어트리뷰트가 있는 클래스
[Deprecated("Use ModernAnimal instead")]
class Animal : Printable {
    private var name: String
    [Range(0, 100)]
    public var age: Int

    public var description: String {
        return "${name} (${age} years old)"
    }

    init(name: String, age: Int) {
        self.name = name
        self.age = age
    }

    public func toString() -> String {
        return description
    }

}