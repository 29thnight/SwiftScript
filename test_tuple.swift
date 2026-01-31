// Test basic tuple literal
let point = (10, 20)
print(point)

// Test named tuple
let namedPoint = (x: 5, y: 15)
print(namedPoint)

// Test tuple member access by index
print(point.0)
print(point.1)

// Test tuple member access by label
print(namedPoint.x)
print(namedPoint.y)

// Test mixed tuple
let mixed = (name: "Alice", 30)
print(mixed.name)
print(mixed.1)

// Test tuple with different types
let person = (name: "Bob", age: 25, active: true)
print(person.name)
print(person.age)
print(person.active)
