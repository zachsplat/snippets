(* basic trie for string lookup. nothing special,
   wrote this for an autocomplete thing *)

type t = {
  mutable is_end : bool;
  children : (char, t) Hashtbl.t;
}

let create () = { is_end = false; children = Hashtbl.create 26 }

let insert trie word =
  let node = ref trie in
  String.iter (fun c ->
    let next = match Hashtbl.find_opt (!node).children c with
      | Some n -> n
      | None ->
        let n = create () in
        Hashtbl.replace (!node).children c n;
        n
    in
    node := next
  ) word;
  (!node).is_end <- true

let search trie word =
  let node = ref trie in
  let found = ref true in
  String.iter (fun c ->
    match Hashtbl.find_opt (!node).children c with
    | Some n -> node := n
    | None -> found := false
  ) word;
  !found && (!node).is_end

let starts_with trie prefix =
  let node = ref trie in
  let ok = ref true in
  String.iter (fun c ->
    if !ok then
      match Hashtbl.find_opt (!node).children c with
      | Some n -> node := n
      | None -> ok := false
  ) prefix;
  if not !ok then []
  else begin
    (* collect all words from this node *)
    let results = ref [] in
    let rec collect node acc =
      if node.is_end then
        results := (List.rev acc |> List.to_seq |> String.of_seq) :: !results;
      Hashtbl.iter (fun c child ->
        collect child (c :: acc)
      ) node.children
    in
    let prefix_chars = List.init (String.length prefix) (fun i -> prefix.[i]) in
    collect !node (List.rev prefix_chars);
    !results
  end

let () =
  let t = create () in
  insert t "hello";
  insert t "help";
  insert t "world";
  assert (search t "hello");
  assert (not (search t "hell"));
  let results = starts_with t "hel" in
  Printf.printf "starts with 'hel': %s\n"
    (String.concat ", " results)
