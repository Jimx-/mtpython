class Number:

    def __init__(self, i):
        self.value = i

    def __add__(self, other):
        return Number(self.value + other.value)

    def __repr__(self):
        return str(self.value)


n1 = Number(1)
n2 = Number(2)
print(n1 + n2)
