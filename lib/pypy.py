def printargs(fn):
    def wrapped(n, pos):
        print(bin(n))
        print(hex(n))
        return fn(n, pos)
    return wrapped


@printargs
def rotate_right(n, pos):
    if pos == 0:
        return n
    else:
        acc = n
        n >>= 1
        n |= ((acc << 7) % 0x100)  # Simulate C behavior of dropping the MSBs
        return rotate_right(n, pos - 1)
