import random

# rules for standard sudoku puzzles
MAX_VALUE = 9
BLOCK_SIZE = 3
LINE_SIZE = 9


def main():
    print grid_to_string(generate_grid())


def grid_to_string(grid):
    row_strings = [''.join(map(str, row)) for row in grid]
    return "\n".join(row_strings)


# list of rows, 9 values per row, 9 rows per grid
def generate_grid(grid=None):
    if grid is None:
        grid = [[]]
    if len(grid[-1]) == LINE_SIZE:
        if len(grid) == LINE_SIZE:
            return grid
        else:
            grid.append([])
            return generate_grid(grid)
    previous_rows = grid[:-1]
    last_row = grid[-1]
    possible_values = possible_next_values(grid)
    random.shuffle(possible_values)
    for next_value in possible_values:
        result = generate_grid(previous_rows + [last_row + [next_value]])
        if result is not None:
            return result
    return None


# given the state so far what values could come next
# returns: list of values valid according to game rules
def possible_next_values(grid):
    permitted_values = set(range(1, MAX_VALUE + 1))
    previous_rows = grid[:-1]
    current_column = len(grid[-1])
    current_row = len(grid) - 1
    column = [row[current_column] for row in previous_rows]
    block = values_in_block(grid, current_column, current_row)
    row = grid[-1]
    return list(permitted_values - set(row + column + block))


# returns a list of the values in the same block as (x, y)
def values_in_block(grid, x, y):
    block_start_x = x - (x % BLOCK_SIZE)
    block_end_x = block_start_x + BLOCK_SIZE
    block_start_y = y - (y % BLOCK_SIZE)
    block_end_y = block_start_y + BLOCK_SIZE
    return [value
            for row in grid[block_start_y:block_end_y]
            for value in row[block_start_x:block_end_x]]


if __name__ == "__main__":
    main()
