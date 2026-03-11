Měsíc visel nad Monakem jako diamant na sametovém nebi. Na střeše hotelu de Paris jsem klečel u trezoru ukrytého ve věžičce apartmánu princezny Delacroix. Zdola bylo slyšet kasino, šampaňské zjevně teklo proudem. V dálce se objevil muž v klobouku. Poznal jsem ho hned. Jmenoval se inspektor Trachta. Zase. Jak se vždycky objeví právě tehdy, když otočím posledním kolečkem?

„Tentokrát tě mám, Le Chat!“ zvolal, když se rozsvítila světla. Jenže já už stál na okraji střechy. „Dokažte to, inspektore,“ řekl jsem a skočil. Zůstal po mně jen stín a vůně kolínské.

O tři noci později. Nice, muzeum šperků. Výstava slavného diamantu Cœur de Lune. Trachta hlídal vchod osobně, nervózně přešlapoval v lakýrkách, které mu zjevně půjčila ambasáda. „Tuhle noc, pánové,“ řekl svým lidem, „kočka skončí v pytli.“

O půlnoci se ozval poplach. Na kameře zahlédli jen rukavici, která přejížděla po skleněné vitríně. Za dvě minuty našli vitrínu prázdnou. A na podlaze vizitku s kresbou kočky v cylindru.

Trachta se rozběhl k přístavu, kde právě startoval člun. „Vzdejte se!“ křičel, ale odpovědí mu byl jen klavírní akord z dálky. Někdo v přilehlé vile hrál La Vie en Rose.

Když ráno noviny psaly o „fantomovi z Azurového pobřeží“, Trachta seděl v kavárně u croissantu, dýmka mu dohořívala. Na stole ležel ubrousek s jedinou větou: „Au revoir, inspektore. Uvidíme se v Paříži.“

## Rozhraní programu

Vaším úkolem je implementovat funkci **plan_heist**, která určí, které ze zadaných trezorů vykrást, aby jste maximalizovali zisk, ale nepřekročili čas pobytu v bance. Návratovou hodnotou je seznam trezorů k vyloupení identifikovaných jejich pořadím na vstupu a pro každý z nich informace, jak ho odemknout (vektor obsahující **true**, pokud je daný kamínek nutné přesunout). Argumenty jsou:

Pole **vaults** popisující jednotlivé trezory. Každý trezor je popsán instancí datového typu **Vault**, která obsahuje:

- Nezáporné číslo **value** – hodnotu věcí v trezoru.
- Nezáporné číslo **pebbles** – počet kamínků v zámku trezoru.
- Pole dvojic **missing_connections** – seznam chybějících spojení mezi kamínky v zámku trezoru (protože chybějících je výrazně méně než přítomných).
Na počátku jsou všechny kamínky zámku na levé straně dveří trezoru. Aby se trezor odemkl, je nutné přesunout **přesně polovinu kamínků** na pravou stranu a to tak aby každý kamínek vpravo byl spojený se všemi kamínky vlevo. Čas na odemčení trezoru je počet přesunutých kamínků. Čas na vlastní vyloupení je zanedbatelný. Některé trezory mohou být rozbité, takže je nelze odemknout.

Nezáporné číslo **transition_time** – čas na přesun od jednoho trezoru k jinému (tento čas je stejný pro všechny dvojice trezorů).

Nezáporné číslo **max_time** – maximální čas, po který můžete setrvat v místnosti s trezory. Víte, že tento čas není příliš velký.

Veškeré výpočty by měly vystačit s rozsahem typu **int**.
