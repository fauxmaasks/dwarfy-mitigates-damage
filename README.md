## Earth Themed RL

#### credits
- [tsoding](https://github.com/tsoding) - nob.h and many other projects and endless knowledge
- [raysan](https://github.com/raysan5/raylib) - for raylib library, scripts, some examples


#### some constraints
- no items
- 1 screen
- no FOV
- no fog

## For Today



## Roadmap-ish?
- [x] use a multimedia lib -> raylib choosen for this one, bit bloated though...
- [ ] proc gen (very minor)
    - [x] make an outer castle resemblance
        - [x] inner palace, columns and pillars, walls
        - [ ] throne room, special expensive stuff, gems and stuff (various effects)
- [ ] player
    - [ ] spells - earth skills
        - [ ] protect with earth (temp shield)
        - [x] move walls -> add some limitation (range, path)
            - [ ] if target small / mid -> push away from the path of the wall
            - [ ] if target big -> stop the wall at tile before the target (launches target?)
            - [ ] if hitting other terrain, propagates the terrain effect (new targeting?)
        - [x] rotate ground, makes entities switch places
            - [ ] how to telegraph this? -> add arrows when on hover (on all tiles afected), highlight the tile selected
            - [x] works on a 3x3, the middle tile rotates the direction the potential monster is shooting
        - [ ] launch - add dmg when landing on existing tile
        - [ ] possible secret mechanic that if u try to bend metal stuff u fail, but after x tries u can (but have to waste turns doing it)
    - [x] movement
- [ ] monsters
    - [ ] spells / Atks
        - [ ] line atk 1 dir
        - [ ] aoe? (how to deal with it?)
        - [ ] cone
    - [ ] different AIs
    - [ ] passives? prob not
    - [ ] resistances? prob not
- [ ] neutrals
    - [ ] stand still xD
- [ ] "animation"
    - [ ] telegraphed atks (when any)
    - [ ] atks execution (atk or telegraphed atk)
#### Nice to haves
- [ ] add some scenery 
    - [ ] gain more points if protect valuables (vases, windows?, statues!) 
    - [ ] houses instead of pillars?
- [ ] actions logs
- [ ] neutrals ai enhancements
    - [ ] run away and try not to get hit?
    - [ ] diff AIs (coward vs protector)
