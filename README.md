# Token based extensible message parser

## Notice

This is a proof of concept application to demonstrate a principal in the architecture of parsing. It does not have a framework and has not been written to be efficient or to production standards.

## Terminology

*Extraction field*: Some unique, usually labeled, portion of a message to be parsed and extracted such as `interface:eth0` or `interface=eth0` where the field would be `interface` and it's value would be `eth0`. In the wider context a field can be simply positional. An example of this would a CEF message where field values are distinguished by the order they appear in and their delineator.

## How it works

The parser works on the principal that if a regular expression can uniquely identify a message type, then the only things that remains is to extract the field values.

Fields are represented by individual regex patterns, and the pattern groups these together in an ordered list. When the pattern is extended by inheritance the new pattern inherits the original pattern's field list, adding it's own fields as needed!

If a property is declared in a pattern, and the pattern ios then inherited, the property is also inherited and will be asserted and triggered if the pattern matches.

The parser uses `hyperscan` to identify that an anchor pattern matches, then uses the index from that to try the rule. It will asses the more complex patterns first, then on failure fall back to less complex ones. A massive efficiency gain could be created by using hyprescan to match the entire pattern, then scheduling the RE2 or PCRE2 to extract the tokens.

Conditional logic and properties are only evaluated one a pattern has matched.

### Unit tests

unite tests can be run by executing the command 

```shell
$ ./test/build/test_parser
```

## Build requirements

The project requiresthe following to be instaled and correctly configured 

- [Google test](https://github.com/google/googletest) 
- [Google benchmark](https://github.com/google/benchmark) though to be fair at the moment [Google benchmark](https://github.com/google/benchmark) isn't doing very much.
- [yaml-cpp](https://github.com/jbeder/yaml-cpp): This is a head only solution, so it would be possible to simply copy the dependancy intp the project and modify the makefile
- [JSON C++ library](https://github.com/nlohmann/json): This is also a single hedaer solution and the above also applies here.


Both these libraries have their own requirements, but these are minimal in so far as they require `pthreads` to be installed and to link.

## Limitations

- All rules are hierarchical and patterns cannot be reused by other rules.
- There is no notion of recursion or of a parser fragment.
- The `YamlFile` class does not do any error checking outside the extent of throwing an exception on errors.
- No account is taken of fields that appear more than once.

## Running the example

The rule definition in `rules/CheckPoint/CheckPoint.yaml` is a rule that

1. Classifies the message `CheckPoint 9929 - [action:"Accept"; conn_direction:"Outgoing"; contextnum:"5"; flags:"7263232"; ifdir:"outbound"; ifname:"eth2"; logid:"0"; loguid:"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}"; origin:"<IP>"; originsicname:"CN=Ord1fw1b,O=bos1cpmgmt1.aspect.com.2twi3g"; sequencenum:"110"; time:"1630417161"; version:"5"; __policy_id_tag:"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\]"; context_num:"1"; dst:"<IP>"; hll_key:"3383197236451420632"; layer_name:"<name>"; layer_name:"<name>"; layer_uuid:"9d7748a0-845f-491d-9eef-1fb41680bc35"; layer_uuid:"3866c7f4-f88d-402e-abde-252a2426d1d7"; match_id:"48"; match_id:"16777222"; parent_rule:"0"; parent_rule:"0"; rule_action:"Accept"; rule_action:"Accept"; rule_name:"Outbound access"; rule_uid:"8bf81033-b6c7-44fe-a88a-2068c155f50e"; rule_uid:"4eb09b29-ccc2-4374-b33a-e66660e3916d"; nat_addtnl_rulenum:"0"; nat_rulenum:"400"; product:"VPN-1 & FireWall-1"; proto:"6"; s_port:"44853"; service:"443"; service_id:"https"; src:"<IP>"; xlatedport:"0"; xlatedst:"<IP>"; xlatesport:"28650"; xlatesrc:"<IP>"]`
2. extracts fields form the message

It differentiates based on the value of the field `action` in two sub patterns that assert the events `CheckPoint Accepted` and `CheckPoint Rejected`.

Conceptually, rule structure is hierarchical in line with ideas put forward by [Guy Duchatelet​](gduchatelet@alertlogic.com) in his [Proposed hierarchical content language](https://alertlogic.atlassian.net/wiki/spaces/DPT/pages/3332964353/24+03+2022+-+Parser+Tree+approach+discussion#Proposed-hierarchical-content-language-(link)%3A).

Rules are expressed such that a rule can either

1. Declare a single pattern that generates an event by asserting the name given to the rule
2. Declare a base pattern that can be inherited from, in order to
   1. add tokens
   2. assert values for tokens (the pattern will only match if the token as the asserted value)
   3. assert the name of the pattern as the event - this happens automatically when a sub pattern matches
3. Assert properties if and only if the pattern matches
   1. As dynamically constructed objects based on extracted field values
   2. Through conditional logic as dynamically constructed objects based on extracted field values

The rule definition for the `CheckPoint` message looks like this

```yaml
id: AAE7F45F-DAF0-1004-D61E-00000A03200B
name: CheckPoint
desc: >-
  CheckPoint Rule
patterns:
  - id: AB9107D4-DAF0-1004-D61E-00000A03200B
    name: CheckPoint
    precedence: 100
    desc: >-
      Check point
    type: root
    event: CheckPoint Accepted
    anchor: DA46E37F-E30F-61B0-6288-052037C38628
    # Tokens in this declaration will be evaluated in the order they are specified 'flags' coming before 'action' would cause the pattern fail
    match: sequential
    tokens: [action,direction,contextnum,flags,ifdir,ifname,loguid,sequencenum,time,version,database_tag,layer_uuid,match_id]
    example: "CheckPoint 9929 - [action:\"Accept\"; conn_direction:\"Outgoing\"; contextnum:\"1\"; flags:\"7263232\"; ifdir:\"outbound\"; ifname:\"eth2\"; logid:\"0\"; loguid:\"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}\"; origin:\"<IP>\"; originsicname:\"CN=Ord1fw1b,O=bos1cpmgmt1.aspect.com.2twi3g\"; sequencenum:\"110\"; time:\"1630417161\"; version:\"5\"; __policy_id_tag:\"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]\"; context_num:\"1\"; dst:\"<IP>\"; hll_key:\"3383197236451420632\"; layer_name:\"<name>\"; layer_name:\"<name>\"; layer_uuid:\"9d7748a0-845f-491d-9eef-1fb41680bc35\"; layer_uuid:\"3866c7f4-f88d-402e-abde-252a2426d1d7\"; match_id:\"48\"; match_id:\"16777222\"; parent_rule:\"0\"; parent_rule:\"0\"; rule_action:\"Accept\"; rule_action:\"Accept\"; rule_name:\"Outbound access\"; rule_uid:\"8bf81033-b6c7-44fe-a88a-2068c155f50e\"; rule_uid:\"4eb09b29-ccc2-4374-b33a-e66660e3916d\"; nat_addtnl_rulenum:\"0\"; nat_rulenum:\"400\"; product:\"VPN-1 & FireWall-1\"; proto:\"6\"; s_port:\"44853\"; service:\"443\"; service_id:\"https\"; src:\"<IP>\"; xlatedport:\"0\"; xlatedst:\"<IP>\"; xlatesport:\"28650\"; xlatesrc:\"<IP>\"]"
    # A property in this model applies to a pattern and concatenates values and token values
  - id: 1950E2A8-D49F-1CF4-DB2B-8B37B61CFBC0
    name: CheckPoint Accepted
    desc: >-
      Check point Accepted rule
    precedence: 10
    type: extends
    extends: AB9107D4-DAF0-1004-D61E-00000A03200B
    # If we specify this as a 'random' pattern ALL tokens including inherited ones will be evaluated as location independent, i.e. 'service_id' being first in the message will not matter and the pattern will succeed
    match: sequential
    tokens: [rule_id,protocol,port,service,service_id]
    # An assert acts as a replacement regex and must, when enclosed in brackets, be one - i.e. The regex for 'action' will become "\[action:\"(Accept)\"; after the substitution is done
    assert:
      - token: action
        value: Accept
    properties:
      - property:
        name: event.type
        deliminator: "."
        tokens: [action,service_id]
      - property:
        dynamic: if contextnum >= 3 then threat.level = "danger"
    example: "CheckPoint 9929 - [action:\"Reject\"; conn_direction:\"Outgoing\"; contextnum:\"1\"; flags:\"7263232\"; ifdir:\"outbound\"; ifname:\"eth2\"; logid:\"0\"; loguid:\"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}\"; origin:\"<IP>\"; originsicname:\"CN=Ord1fw1b,O=bos1cpmgmt1.aspect.com.2twi3g\"; sequencenum:\"110\"; time:\"1630417161\"; version:\"5\"; __policy_id_tag:\"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]\"; context_num:\"1\"; dst:\"<IP>\"; hll_key:\"3383197236451420632\"; layer_name:\"<name>\"; layer_name:\"<name>\"; layer_uuid:\"9d7748a0-845f-491d-9eef-1fb41680bc35\"; layer_uuid:\"3866c7f4-f88d-402e-abde-252a2426d1d7\"; match_id:\"48\"; match_id:\"16777222\"; parent_rule:\"0\"; parent_rule:\"0\"; rule_action:\"Accept\"; rule_action:\"Accept\"; rule_name:\"Outbound access\"; rule_uid:\"8bf81033-b6c7-44fe-a88a-2068c155f50e\"; rule_uid:\"4eb09b29-ccc2-4374-b33a-e66660e3916d\"; nat_addtnl_rulenum:\"0\"; nat_rulenum:\"400\"; product:\"VPN-1 & FireWall-1\"; proto:\"6\"; s_port:\"44853\"; service:\"443\"; service_id:\"https\"; src:\"<IP>\"; xlatedport:\"0\"; xlatedst:\"<IP>\"; xlatesport:\"28650\"; xlatesrc:\"<IP>\"]"
  - id: 3927EB41-29B8-C62E-648E-DB5F4C4C2E67
    name: CheckPoint Rejected
    desc: >-
      Check point Rejected rule
    precedence: 10
    type: extends
    extends: AB9107D4-DAF0-1004-D61E-00000A03200B
    match: sequential
    tokens: [rule_id,protocol,port,service,service_id]
    assert:
      - token: action
        value: Reject
    example: "CheckPoint 9929 - [action:\"Reject\"; conn_direction:\"Outgoing\"; contextnum:\"1\"; flags:\"7263232\"; ifdir:\"outbound\"; ifname:\"eth2\"; logid:\"0\"; loguid:\"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}\"; origin:\"<IP>\"; originsicname:\"CN=Ord1fw1b,O=bos1cpmgmt1.aspect.com.2twi3g\"; sequencenum:\"110\"; time:\"1630417161\"; version:\"5\"; __policy_id_tag:\"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]\"; context_num:\"1\"; dst:\"<IP>\"; hll_key:\"3383197236451420632\"; layer_name:\"<name>\"; layer_name:\"<name>\"; layer_uuid:\"9d7748a0-845f-491d-9eef-1fb41680bc35\"; layer_uuid:\"3866c7f4-f88d-402e-abde-252a2426d1d7\"; match_id:\"48\"; match_id:\"16777222\"; parent_rule:\"0\"; parent_rule:\"0\"; rule_action:\"Accept\"; rule_action:\"Accept\"; rule_name:\"Outbound access\"; rule_uid:\"8bf81033-b6c7-44fe-a88a-2068c155f50e\"; rule_uid:\"4eb09b29-ccc2-4374-b33a-e66660e3916d\"; nat_addtnl_rulenum:\"0\"; nat_rulenum:\"400\"; product:\"VPN-1 & FireWall-1\"; proto:\"6\"; s_port:\"44853\"; service:\"443\"; service_id:\"https\"; src:\"<IP>\"; xlatedport:\"0\"; xlatedst:\"<IP>\"; xlatesport:\"28650\"; xlatesrc:\"<IP>\"]"
anchors:
  - anchor:
    id: DA46E37F-E30F-61B0-6288-052037C38628
    name: CheckPoint
    patterns: [^(CheckPoint)]
tokens:
  - token:
    name: action
    regex: \[action:\"([A-Za-z]+)\";
    type: string
  - token:
    name: direction
    regex: conn_direction:\"([A-Za-z]+)\";
    type: string
  - token:
    name: contextnum
    regex: contextnum:\"([0-9]+)\"
    type: int
  - token:
    name: flags
    regex: flags:\"([0-9]+)\"
    type: int
  - token:
    name: ifdir
    regex: ifdir:\"([a-z]+)\"
    type: string
  - token:
    name: ifname
    regex: ifname:\"([a-z0-9]+)\"
    type: string
  - token:
    name: loguid
    regex: loguid:\"([\{\}a-z0-9,]+)\"
    type: string
  - token:
    name: sequencenum
    regex: sequencenum:\"([0-9]+)\";
    type: int
  - token:
    name: time
    regex: time:\"([0-9]+)\";
    type: int
  - token:
    name: version
    regex: version:\"([0-9]+)\";
    type: int
  - token:
    name: database_tag
    regex: \[db_tag=\{([0-9A-F\-]+)\};
    type: string
  - token:
    name: layer_uuid
    regex: layer_uuid:\"([0-9\-a-f]+)\"
    type: string
  - token:
    name: match_id
    regex: match_id:\"([0-9]+)\";
    type: int
  - token:
    name: rule_id
    regex: rule_uid:\"([0-9a-f\-]+)\";
    type: string
  - token:
    name: protocol
    regex: proto:\"([0-9]+)\";
    type: int
  - token:
    name: port
    regex: s_port:\"([0-9]+)\";
    type: int
  - token:
    name: service
    regex: service:\"([0-9]+)\";
    type: int
  - token:
    name: service_id
    regex: service_id:\"(https|http|scp)\";
    type: string
```

## How the rule works

When a pattern is subclassed it automatically inherits all the tokens declared in the base pattern, and may add specific tokens of its own. The sub pattern can then add asserts for any token values declared by the base pattern or itself. It currently does not inherit the base patterns asserts (though this would be relatively trivial to implement).

All tokens are local to the rules file.

This currently works onn the basis that

1. All tokens in a rule execute the `Regex` for the token only once
   1. patterns that share tokens share the token object as an `std::shared_ptr`.
   2. when a match is found for a token is set to `matched`.
   3. When an `assert` is found the regex is substituted to create a new more specific regex pattern for that search. The underlying regex is not changed.
   4. If a subclassed pattern matches, but the assert fails, then another subclassed pattern, which declares a different `assert` is assessed for the same token.
2. More complex pattern are evaluated first.
   1. Patterns sorted to guaranty that specific patterns are tried over generic ones
      1. for complexity (number of tokens)
      2. then by precedence
3. Properties can be specified either as
   1. Unconditional dynamically constructed properties that execute what the pattern matches
   2. Conditional properties that execute if the condition evaluates to `true`
      1. These have the form `if` `then` `else` which can use `or` in the conditional clause and `and` in the assignment clauses
      2. The parser uses space as the delineator in the expression elements
         1. the clause `if device_id == "toy"` is valid
         2. the clause `if device_id=="toy"` is not
   3. Note that a property (wether it si dynamic or not) will only be evaluated if (and only if) the pattern matches. If an assert (as in pattern `1950E2A8-D49F-1CF4-DB2B-8B37B61CFBC0` above) states that that `action` must equal `Accept`, the pattern will only match when the field value for `action` is `Accept`.

## Dynamic properties based on conditional logic

Dynamic logic can be used to assert property values by attaching a dynamic property clause to the pattern. Properties are inherited when the pattern is extended, as in the bellow example.

```yaml
iid: 5F175E9A-5C85-1176-810A-89A9E5CBDEA1
name: AWS Toy
desc: >-
  AWS Toy
patterns:
  - id: 4B5D4C39-199F-1432-8A2F-93DEBF110E1
    name: AWS Toy
    precedence: 100
    desc: >-
      AWS Toy
    type: root
    anchor: DA46E37F-E30F-61B0-6288-052037C38628
    match: sequential
    # Tokens and properties will be inherited by anything pattern that extends this pattern
    tokens: [vendor,device_id,user,ip.incoming,file_name,event]
    properties:
      - property:
        dynamic: if device_id == "toy" and file_name ?= "security" then even.name = vendor + "." + device_id and event.string = event + " to " + user else event.name = "AWS event"
    example:
      "aws toy: {user:barry,ip.incoming:192.168.167.12,file:/home/barry/security/permissions.cfg,event:Access denied}"
```

The example above asserts two properties `event.name` and `event.string`. `event.name` will always be asserted if the pattern (or any derived pattern) matches. `event.string` is only asserted if the condition `device_id == "toy" and file_name ?= "security"` is true.

The following messages (taken from `logs/CheckPoint.log` will both trigger the `event.name` and the `event.string` property based on the above rule because the extracted field `file_name` contains the string `security` and the `device_id` field evaluates to `toy`.

```json
aws toy: {user:fred,ip.incoming:192.168.167.12,file:/home/barry/security/permissions.cfg,event:Access denied}
aws toy: {user:barry,ip.incoming:192.168.167.12,file:/home/barry/security/permissions.cfg,event:Access granted}
aws toy: {user:fred,ip.incoming:192.168.167.12,file:/home/barry/personal/lifehacks.md,event:Access granted}
aws toy: {user:barry,ip.incoming:192.168.236.13,file:/home/barry/personal/lifehacks.md,event:Access denied}
aws toy: {user:tom,ip.incoming:10.10.1.65,file:/home/tom/workspace/runthis.c,event:Access denied}
```

For those messages, they produce the output

```json
{
    "parsed": [
        {
            "event": {
                "name": "AWS Toy access denied",
                "pattern_id": "2DAC1F35-864B-C244-A9D9-C3F4883423D8"
            },
            "property": {
                "even.name": "aws.toy",
                "event.string": "Access denied to fred"
            },
            "rule": {
                "id": "5F175E9A-5C85-1176-810A-89A9E5CBDEA1",
                "name": "AWS Toy"
            },
            "tokens": {
                "device_id": "toy",
                "event": "Access denied",
                "file_name": "/home/barry/security/permissions.cfg",
                "ip.incoming": "192.168.167.12",
                "user": "fred",
                "vendor": "aws"
            }
        },
        {
            "event": {
                "name": "AWS Toy access granted",
                "pattern_id": "3927EB41-29B8-C62E-648E-DB5F4C4C2E67"
            },
            "property": {
                "even.name": "aws.toy",
                "event.string": "Access granted to barry"
            },
            "rule": {
                "id": "5F175E9A-5C85-1176-810A-89A9E5CBDEA1",
                "name": "AWS Toy"
            },
            "tokens": {
                "device_id": "toy",
                "event": "Access granted",
                "file_name": "/home/barry/security/permissions.cfg",
                "ip.incoming": "192.168.167.12",
                "user": "barry",
                "vendor": "aws"
            }
        },
        {
            "event": {
                "name": "AWS Toy access granted",
                "pattern_id": "3927EB41-29B8-C62E-648E-DB5F4C4C2E67"
            },
            "property": {
                "even.name": "aws.toy",
                "event.string": "Access granted to fred"
            },
            "rule": {
                "id": "5F175E9A-5C85-1176-810A-89A9E5CBDEA1",
                "name": "AWS Toy"
            },
            "tokens": {
                "device_id": "toy",
                "event": "Access granted",
                "file_name": "/home/barry/personal/lifehacks.md",
                "ip.incoming": "192.168.167.12",
                "user": "fred",
                "vendor": "aws"
            }
        },
        {
            "event": {
                "name": "AWS Toy access denied",
                "pattern_id": "2DAC1F35-864B-C244-A9D9-C3F4883423D8"
            },
            "property": {
                "even.name": "aws.toy",
                "event.string": "Access denied to barry"
            },
            "rule": {
                "id": "5F175E9A-5C85-1176-810A-89A9E5CBDEA1",
                "name": "AWS Toy"
            },
            "tokens": {
                "device_id": "toy",
                "event": "Access denied",
                "file_name": "/home/barry/personal/lifehacks.md",
                "ip.incoming": "192.168.236.13",
                "user": "barry",
                "vendor": "aws"
            }
        },
        {
            "event": {
                "name": "AWS Toy access denied",
                "pattern_id": "2DAC1F35-864B-C244-A9D9-C3F4883423D8"
            },
            "property": {
                "even.name": "aws.toy",
                "event.string": "Access denied to tom"
            },
            "rule": {
                "id": "5F175E9A-5C85-1176-810A-89A9E5CBDEA1",
                "name": "AWS Toy"
            },
            "tokens": {
                "device_id": "toy",
                "event": "Access denied",
                "file_name": "/home/tom/workspace/runthis.c",
                "ip.incoming": "10.10.1.65",
                "user": "tom",
                "vendor": "aws"
            }
        }
    ]
}
```

## Formatting JSON output

JSON output is unformatted. You can format output, say into a file, by running the output through `jq`. In  `Linux` or MacOS this can be achieved by piping

```bash
  $ parser -r rules/ -l logs/ | jq > parsed.json
```

This will produce the file [parsed.json](parsed.json)


## Further work

1. add the notion of `normalized` tokens values by creating a global token store that can be used to normalize local rule tokens to global system tokens durning which phase value translation and mapping should take place.
   1. Add a facility to map `rule` token values to `normalized` system token values.
   2. Add a facility to map `normalized` rules token values to `system` tokens.
2. fix validation for rules via example messages.

## Unit tests

The unit tests in `test/src` use Google Test and can be run by calling `make runtest`. They exercise the main body of the code, and load test file then used the resulting rule to parse the test message.

## Command line

The POC parser is setup to read in a database of rules as a file structure of `*.yaml` files and log files as a directory structure of `*.log` files.

```bash
  $ ./build/parser -r rules -l logs
```

Will parse the content of all log files in `log`, applying all rules in `rules`

You can a single test message by running

```bash
   $ ./build/parser -r rules -m <message_text>
```

as in

`./build/parser -r rules -m 'CheckPoint 9929 - [action:"Accept"; conn_direction:"Outgoing"; contextnum:"5"; flags:"7263232"; ifdir:"outbound"; ifname:"eth2"; logid:"0"; loguid:"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}"; origin:"<IP>"; originsicname:"CN=Ord1fw1b,O=bos1cpmgmt1.aspect.com.2twi3g"; sequencenum:"110"; time:"1630417161"; version:"5"; __policy_id_tag:"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\]"; context_num:"1"; dst:"<IP>"; hll_key:"3383197236451420632"; layer_name:"<name>"; layer_name:"<name>"; layer_uuid:"9d7748a0-845f-491d-9eef-1fb41680bc35"; layer_uuid:"3866c7f4-f88d-402e-abde-252a2426d1d7"; match_id:"48"; match_id:"16777222"; parent_rule:"0"; parent_rule:"0"; rule_action:"Accept"; rule_action:"Accept"; rule_name:"Outbound access"; rule_uid:"8bf81033-b6c7-44fe-a88a-2068c155f50e"; rule_uid:"4eb09b29-ccc2-4374-b33a-e66660e3916d"; nat_addtnl_rulenum:"0"; nat_rulenum:"400"; product:"VPN-1 & FireWall-1"; proto:"6"; s_port:"44853"; service:"443"; service_id:"https"; src:"<IP>"; xlatedport:"0"; xlatedst:"<IP>"; xlatesport:"28650"; xlatesrc:"<IP>"]' | jq`

This should give you the output

```json
{
  "parsed": [
    {
      "event": {
        "name": "CheckPoint Accepted",
        "pattern_id": "1950E2A8-D49F-1CF4-DB2B-8B37B61CFBC0"
      },
      "property": {
        "event.type": "Accept.https",
        "threat.level": "danger"
      },
      "rule": {
        "id": "AAE7F45F-DAF0-1004-D61E-00000A03200B",
        "name": "CheckPoint"
      },
      "tokens": {
        "action": "Accept",
        "contextnum": 5,
        "database_tag": "599255D7-8B94-704F-9D9A-CFA3719EA5CE",
        "direction": "Outgoing",
        "flags": 7263232,
        "ifdir": "outbound",
        "ifname": "eth2",
        "layer_uuid": "9d7748a0-845f-491d-9eef-1fb41680bc35",
        "loguid": "{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}",
        "match_id": 48,
        "port": 44853,
        "protocol": 6,
        "rule_id": "8bf81033-b6c7-44fe-a88a-2068c155f50e",
        "sequencenum": 110,
        "service": 443,
        "service_id": "https",
        "time": 1630417161,
        "version": 5
      }
    }
  ]
}
```

Note that the property in `CheckPoint Accepted` and that the value of `contextnum:"5";` now sets the property values

```json
"property": {
        "event.type": "Accept.https",
        "threat.level": "danger"
      },
```

The same command with the value `action:"Reject";` in the message string will trigger a derived rule and produce the result (with the same tokens but without the dynamic properties)

```json
{
  "parsed": [
    {
      "event": {
        "name": "CheckPoint",
        "pattern_id": "AB9107D4-DAF0-1004-D61E-00000A03200B"
      },
      "rule": {
        "id": "AAE7F45F-DAF0-1004-D61E-00000A03200B",
        "name": "CheckPoint"
      },
      "tokens": {
        "action": "Rejected",
        "contextnum": 5,
        "database_tag": "599255D7-8B94-704F-9D9A-CFA3719EA5CE",
        "direction": "Outgoing",
        "flags": 7263232,
        "ifdir": "outbound",
        "ifname": "eth2",
        "layer_uuid": "9d7748a0-845f-491d-9eef-1fb41680bc35",
        "loguid": "{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}",
        "match_id": 48,
        "sequencenum": 110,
        "time": 1630417161,
        "version": 5
      }
    }
  ]
}
```