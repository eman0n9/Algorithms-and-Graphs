Bylo chladné ráno 3. listopadu 1420 krajového letopočtu. Mlha ležela nízko nad poli a cesty Kraje ztichly, jako by samy zadržely dech. Na loukách u Povodí se shromáždili hobiti, muži i ženy, mladí i staří. Kraj byl zpustošen a už nebylo na co čekat.

V čele stál Smělmír Brandorád, po jeho boku Peregrin Bral, oba zoceleni válkou o Prsten. Po jejich stranách hobiti a hobitky, rozhodnutí chránit domovy, které kdysi plnily písní a smíchem. Nyní v rukou svíraly praky, hole a vidle. Jak nezvyklá podívaná.

Smíšek s Pipinem rozmístili své lidi po svazích bývalé pískovny. Když první světlo dne dopadlo na mokré trsy trávy, zaznělo tiché znamení. Z houští vyletěly kameny, a než se vetřelci vzpamatovali, hobiti se na ně sesypali ze všech stran. Sarumanovi muži, zaskočení zuřivostí malých postav, klopýtali v bahně a ustupovali.

Hobitky bojovaly po boku svých druhů; přesně mířily, ošetřovaly raněné, volaly povely, když ostatní váhali. Bitva trvala krátce, ale byla prudká. Devatenáct hobitů padlo a sedmdesát cizinců zůstalo ležet v prachu. Ti, kteří přežili, složili zbraně nebo prchli z Kraje.

Po bitvě Frodo chodil mezi zraněnými a přikazoval, aby nebylo pomsty. „Země, která se brání, nesmí přijmout krutost svých nepřátel,“ pravil tiše.

Tak skončila Bitva u Povodí, druhá a poslední v dějinách Kraje. Nebyla velká v měřítku králů, ale pro hobity znamenala víc než všechny války světa, protože v ten den stáli muži i ženy bok po boku a vybojovali zpět svůj domov i mír, který jim byl vzat.

## Rozhraní programu

Vaším úkolem je implementovat třídu **HobbitArmy**, která musí obsahovat:

Metodu **add**, která přidá hobita do armády. Pokud již armáda hobita daného jména obsahuje nebo přidávaný hobit nemá kladný počet životů, metoda vrátí **false** a armáda zůstane nezměněná.

Metodu **erase**, která dostane jméno hobita a odebere ho z armády. Návratovou hodnotou je odebraný hobit, případně prázdný **optional**, pokud se hobit daného jména v armádě nenacházel.

Metodu **stats**, která vrátí aktuální stav hobita zadaného jména, případně prázdný **optional**, pokud se hobit daného jména v armádě nenachází.

Metodu **for_each**, která dostane jako argument funktor, který postupně v abecedním pořadí zavolá na všech hobitech v armádě. Tato metoda slouží ke čtení a neupravuje armádu, tedy funktoru můžete předávat hobity **const** referencí.

Metodu **enchant**, která dostane jména dvou hobitů **first** a **last** a hodnoty, o které má změnit vlastnosti, a upraví vlastnosti všech hobitů v armádě jejichž jména jsou abecedně v intervalu **first** až **last** (včetně obou mezí). Pokud je **first** větší než **last**, metoda nic neudělá a vrátí **true**.

Pokud by úpravou klesly životy některého z hobitů na **0 nebo méně**, není možné danou úpravu provést, metoda nesmí upravit žádného hobita a musí vrátit **false**. Efektivní implementace této funkcionality je obtížná, proto je konstanta **CHECK_NEGATIVE_HP** nastavena na **false**.

Statickou konstantu **CHECK_NEGATIVE_HP** typu **bool**. Pokud je **true** jsou zadávány i testy, kdy může metoda **enchant** vrátit **false**. Pokud je **false**, je garantováno, že metoda **enchant** nebude mít důvod selhat za cenu bodové penalizace.
