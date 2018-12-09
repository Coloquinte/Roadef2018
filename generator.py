#!/usr/bin/python3

from collections import namedtuple
import random

Item = namedtuple('Item', ['length', 'width'])
Defect = namedtuple('Defect', ['x', 'y', 'width', 'height'])
Params = namedtuple('Params', ['nb_stacks', 'avg_stack_size', 'avg_defects'])

min_waste = 20
min_xx = 100
min_yy = 100
max_xx = 3500
plate_width = 6000
plate_height = 3210
nb_plates = 100

def fits(a, b):
    return a == b or a + min_waste <= b

def valid_item(item):
    if item.width < min_waste or item.length < min_waste:
        return False
    if item.width > max_xx or item.length > max_xx:
        return False
    if not fits(item.width, plate_height) and not fits(item.length, plate_height):
        return False
    return True

def valid_defect(defect):
    if defect.x < 0 or defect.y < 0:
        return False
    if defect.width < 0 or defect.height < 0:
        return False
    if defect.x + defect.width > plate_width:
        return False
    if defect.y + defect.height> plate_height:
        return False
    return True

class ProblemGenerator(object):
    def __init__(self, params):
        self.stacks = []
        self.plates = []
        self.params = params

    def generate_defects(self):
        for i in range(nb_plates):
            nb_defects = random.randint(1, 2 * params.avg_defects)
            self.generate_one_plate(nb_defects)

    def generate_items(self):
        for i in range(params.nb_stacks):
            nb_items = random.randint(1, 2 * params.avg_stack_size)
            self.generate_one_stack(nb_items)

    def write(self, prefix):
        batch_name = prefix + "_batch.csv"
        defects_name = prefix + "_defects.csv"
        with open(batch_name, 'w') as f:
            f.write("ITEM_ID;LENGTH_ITEM;WIDTH_ITEM;STACK;SEQUENCE\n")
            item_id = 0
            for stack_id, stack in enumerate(self.stacks):
                for seq_id, item in enumerate(stack):
                    vals = [item_id, item.length, item.width, stack_id, seq_id + 1]
                    f.write(";".join([str(v) for v in vals]) + "\n")
                    item_id += 1
        with open(defects_name, 'w') as f:
            f.write("DEFECT_ID;PLATE_ID;X;Y;WIDTH;HEIGHT\n")
            defect_id = 0
            for plate_id, plate in enumerate(self.plates):
                for defect in plate:
                    vals = [defect_id, plate_id, defect.x, defect.y, defect.width, defect.height]
                    f.write(";".join([str(v) for v in vals]) + "\n")
                    defect_id += 1

    def generate_one_stack(self, nb_items):
        stack = [self.generate_one_item() for i in range(nb_items)]
        self.stacks.append(stack)

    def generate_one_plate(self, nb_defects):
        plate = [self.generate_one_defect() for i in range(nb_defects)]
        self.plates.append(plate)

    def generate_one_item(self):
        while True:
            length = random.randint(min_waste, max_xx)
            width = random.randint(min_waste, max_xx)
            item = Item(length, width)
            if valid_item(item):
                return item

    def generate_one_defect(self):
        while True:
            x = random.randint(0, plate_width)
            y = random.randint(0, plate_height)
            width = random.randint(0, 20)
            height = random.randint(0, 20)
            defect = Defect(x=x, y=y, width=width, height=height)
            if valid_defect(defect):
                return defect

params = Params(nb_stacks=300, avg_stack_size=3, avg_defects=10)
gen = ProblemGenerator(params)
gen.generate_items()
gen.generate_defects()
gen.write("generated/G1")
