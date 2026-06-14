## build steps

conan install . --output-folder=build --build=missing

cd build

cmake ..

make

# HTTP API for player server

## Suitable encoding/parsing for card + player move needs to be chosen and doc

The player server needs to implement:
```
HTTP POST:
path:
  /next_move:
    post:
      summary: Request for move
      description: The poker game central server will make a HTTP Post request to the player server when it's their turn, with the recorded state and history, and retrieve their action.

      requestBody:
        required: true
        content:
          application/json:
            schema:
              type: object
              required:
                - board
                - history
                - position
              properties:
                board:
                  type: string[]
                  example: ["A1", "A2", "A3"]
                  note: "A1" = Ace of heart.
                history:
                  type: string[]
                  example: ["1,R20", "2C", "3F"]
                  note: Encode the players' unique id + move. Move is encoded to be: 1st char for move type, remaining char is the raised amount for raise.
                position:
                  type: string
                  example: "1,BB"
                  note: Contain the player id + position.
      responses:
        '201':
          description: Player's move
          content:
            application/json:
              schema:
                type: string
                example: "R200" | "C" | "F" | "A200"
                note: To keep it simple for the history, the all-in should have the amount of money worth as well.
        '400':
          description: Invalid request body
        '500':
          description: Internal server error
```
