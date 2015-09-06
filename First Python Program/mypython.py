#!/usr/bin/python3

import random
import string

def random_letters():
    return ''.join(random.choice(string.ascii_lowercase) for x in range(10))

def random_integer():
    return random.randint(1,42)

random1 = random_letters()
random2 = random_letters()
random3 = random_letters()

print("mypython.py:")
print("------------")

print("\nFile contents:")
file1 = open("my_py_file1", "w")
file1.write(random1)
print(random1)
file1.close()

file2 = open("my_py_file2", "w")
file2.write(random2)
print(random2)
file2.close()

file3 = open("my_py_file3", "w")
file3.write(random3)
print(random3)
file3.close()

print("\nRandom numbers:")
randomInt1 = random_integer()
print(randomInt1)

randomInt2 = random_integer()
print(randomInt2)

print("\nTheir product:")
product = randomInt1 * randomInt2
print(product)
