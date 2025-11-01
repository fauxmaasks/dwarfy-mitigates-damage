## Earth Themed RL

#### credits
- [tsoding](https://github.com/tsoding) - nob.h and many other projects and endless knowledge
- [raysan](https://github.com/raysan5/raylib) - for raylib library, scripts, some examples


#### some constraints
- no items
- 1 screen
- no FOV
- no fog

## Next ups
- make entity add function (choose sensible place to put them in)

### things to think about
- less spells but more interaction
- 1 spell that does everything?
- what would a puzzle be like?

## Roadmap-ish?
- [x] use a multimedia lib -> raylib choosen for this one, bit bloated though...
- [ ] proc gen (very minor)
    - [x] make an outer castle resemblance
        - [x] inner palace, columns and pillars, walls (perhaps more of a barn theme? inside fortress?)
        - [ ] throne room, special expensive stuff, gems and stuff (various effects)
- [~] player
    - [~] spells - earth skills
        - [x] move walls -> add some limitation (range, path)
            - [x] if target -> stop the wall at tile before the target (launches target?)
            - [ ] also telegraph this (show the path)
            - [ ] Separate spell? (or if 4 tiles+ moved) if hitting other terrain, propagates the terrain effect (new targeting?)
        - [x] rotate ground, makes entities switch places
            - [x] how to telegraph this? -> add arrows when on hover (on all tiles afected), highlight the tile selected
                - [ ] improve a bit, highlight all tiles and maybe draw arrows on all of them?
            - [x] works on a 3x3, the middle tile rotates the direction the potential monster is shooting
        - [x] create wall - walls block atks (mostly dragon and cyclop for now)
            - [ ] minor telegraph?
            - [ ] launch - if creating wall on existing tile, make target jump to random tile adjacent
        - [ ] add ranges
    - [x] movement
- [ ] monsters
    - [ ] spells / Atks
        - [x] line atk 1 dir, until end of lvl or wall
        - [x] aoe
        - [x] rectangle 
        - [ ] future: cone (if I ever learn how to do them)
    - [ ] different AIs?
        - [ ] ai could be simply finding other targets to hit
        - [ ] move closer?
- [ ] neutrals
    - [ ] stand still, take dmg at turn end
- [ ] make debug mode with editor? make it easier to save load levels
    - bit overkill... but would be nice to work, prob leave for a future iteration
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

#### Others
- [ ] possible secret mechanic that if u try to bend metal stuff u fail, but after x tries u can (but have to waste turns doing it)
- [ ] other limitations? what if no limitations? 
    I think some is needed, perhaps move terrain only goes 2 tiles? 
    unless other stuff applies (diff terrain, diff conditions, wind, enemies, etc)
