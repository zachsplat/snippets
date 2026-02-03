(* minimal json parser. supports objects, arrays, strings, numbers, bools, null.
   good enough for config files *)

type json =
  | Null
  | Bool of bool
  | Number of float
  | String of string
  | Array of json list
  | Object of (string * json) list

exception Parse_error of string

let is_ws c = c = ' ' || c = '\t' || c = '\n' || c = '\r'

let parse s =
  let len = String.length s in
  let pos = ref 0 in
  let peek () = if !pos < len then s.[!pos] else '\000' in
  let next () = let c = peek () in incr pos; c in
  let skip_ws () = while !pos < len && is_ws (peek ()) do incr pos done in

  let rec parse_value () =
    skip_ws ();
    match peek () with
    | '"' -> parse_string ()
    | '{' -> parse_object ()
    | '[' -> parse_array ()
    | 't' -> ignore (next ()); ignore (next ()); ignore (next ()); ignore (next ()); Bool true
    | 'f' -> ignore (next ()); ignore (next ()); ignore (next ()); ignore (next ()); ignore (next ()); Bool false
    | 'n' -> ignore (next ()); ignore (next ()); ignore (next ()); ignore (next ()); Null
    | c when c = '-' || (c >= '0' && c <= '9') -> parse_number ()
    | c -> raise (Parse_error (Printf.sprintf "unexpected '%c' at %d" c !pos))

  and parse_string () =
    ignore (next ());  (* skip opening quote *)
    let buf = Buffer.create 32 in
    let rec loop () =
      match next () with
      | '"' -> String (Buffer.contents buf)
      | '\\' ->
        (match next () with
         | 'n' -> Buffer.add_char buf '\n'
         | 't' -> Buffer.add_char buf '\t'
         | '"' -> Buffer.add_char buf '"'
         | '\\' -> Buffer.add_char buf '\\'
         | c -> Buffer.add_char buf c);
        loop ()
      | c -> Buffer.add_char buf c; loop ()
    in
    loop ()

  and parse_number () =
    let start = !pos in
    if peek () = '-' then incr pos;
    while !pos < len && peek () >= '0' && peek () <= '9' do incr pos done;
    if !pos < len && peek () = '.' then begin
      incr pos;
      while !pos < len && peek () >= '0' && peek () <= '9' do incr pos done
    end;
    Number (float_of_string (String.sub s start (!pos - start)))

  and parse_array () =
    ignore (next ());  (* [ *)
    skip_ws ();
    if peek () = ']' then (ignore (next ()); Array [])
    else begin
      let items = ref [parse_value ()] in
      skip_ws ();
      while peek () = ',' do
        ignore (next ());
        items := parse_value () :: !items;
        skip_ws ()
      done;
      ignore (next ());  (* ] *)
      Array (List.rev !items)
    end

  and parse_object () =
    ignore (next ());  (* { *)
    skip_ws ();
    if peek () = '}' then (ignore (next ()); Object [])
    else begin
      let pairs = ref [] in
      let parse_pair () =
        skip_ws ();
        match parse_value () with
        | String key ->
          skip_ws ();
          ignore (next ());  (* : *)
          let v = parse_value () in
          pairs := (key, v) :: !pairs
        | _ -> raise (Parse_error "expected string key")
      in
      parse_pair ();
      skip_ws ();
      while peek () = ',' do
        ignore (next ());
        parse_pair ();
        skip_ws ()
      done;
      ignore (next ());  (* } *)
      Object (List.rev !pairs)
    end
  in
  let result = parse_value () in
  result

let () =
  let j = parse {|{"name": "test", "values": [1, 2, 3], "ok": true}|} in
  match j with
  | Object _ -> Printf.printf "parsed ok\n"
  | _ -> Printf.printf "unexpected\n"
