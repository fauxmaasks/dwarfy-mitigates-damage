## Earth Themed RL

#### working on...
- [x] adding checks if room is able to fit
- [x] add tunneler turn chance (need to improve though)
- [ ] improve corridors shape, prevent it from going over other tunnels
- [ ] add tunneler queues (first dig tunnels then rooms?)
- [ ] add room tunnelers that connect to other tunnels/rooms
- [ ] add secret "shortcuts"

#### credits
- [tsoding](https://github.com/tsoding) - nob.h and many other projects and endless knowledge
- [raysan](https://github.com/raysan5/raylib) - for raylib library, scripts, some examples


#### some constraints
- no items
- 1 screen
- no FOV
- no fog

## For Today
- Proc Gen
    - Tunnelers


## Roadmap-ish?
- [x] use a multimedia lib -> raylib choosen for this one, bit bloated though...
- [ ] proc gen
    - [x] make a castle like dungeon
        - [x] inner palace, columns and pillars, walls
        - [ ] throne room, special expensive stuff, gems and stuff (various effects)
    - [ ] add some prefabs, can be small to mid sized
- [ ] actions logs
- [ ] player
    - [ ] spells - earth skills
        - [ ] protect with earth (temp shield)
        - [x] move walls -> add some limitation (range, path)
        - [ ] rotate ground, makes entities switch places
            - [ ] how to telegraph this? 
            - [ ] works on a 3x3, the middle tile rotates the direction the potential monster is shooting
        - [ ] launch
    - [x] movement
- [ ] monsters
    - [ ] spells / Atks
        - [ ] line atk 1 dir
        - [ ] aoe? (how to deal with it?)
        - [ ]
    - [ ] different AIs
    - [ ] passives? prob not
    - [ ] resistances? prob not
- [ ] neutrals
    - [ ] run away and try not to get hit?
    - [ ] diff AIs (coward vs protector)
- [ ] animation
    - [ ] telegraphed atks (when any)
    - [ ] atks execution (atk or telegraphed atk)
