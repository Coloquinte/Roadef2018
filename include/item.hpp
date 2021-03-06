// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef ITEM_HPP
#define ITEM_HPP

struct Item {
  int id;
  int width;
  int height;
  int stack;
  int sequence;

  int area() const {
    return width * height;
  }
};

#endif

