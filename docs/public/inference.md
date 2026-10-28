# Inference behavior

The converter generates a draft-07 schema from one JSON value. Every result
includes:

```json
{"$schema":"http://json-schema.org/draft-07/schema#"}
```

The actual result also contains the type-specific keywords described below.

## Objects

Objects produce `type: object` and a `properties` entry for each observed key.
All observed keys are listed in `required`. For an empty object, `properties`
is empty and `required` is omitted.

The converter does not set `additionalProperties: false`, so the schema does
not prohibit extra properties.

## Arrays

- Empty arrays produce `items: {}`, allowing any item type.
- A one-element array uses that element's schema directly as `items`.
- Arrays with multiple elements use `items: {"anyOf": [...]}`, with one schema
  for each observed element.

For example, `[1, null]` produces:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "array",
  "items": {
    "anyOf": [
      {"type": "integer"},
      {"type": "null"}
    ]
  }
}
```

Equivalent element schemas are not deduplicated. Object shapes in an array are
retained as separate alternatives, not merged into one object definition.
Array lengths and element positions are not constrained.

## Primitive values

| Input | Inferred type |
| --- | --- |
| String | `string` |
| `true` or `false` | `boolean` |
| `null` | `null` |
| Number without a decimal point or exponent | `integer` |
| Number with a decimal point or exponent | `number` |

Numeric classification uses the JSON spelling. For example, `1` becomes
`integer`, while `1.0` and `1e0` become `number`.

## Limitations

The converter does not infer string formats, patterns, numeric ranges, enums,
defaults, descriptions, or optional properties from one example. It does not
validate other documents against the generated schema. Review and refine the
output according to your application's contract.

Both parser and converter limit nesting to 64 levels. Available token and
output capacity also limit the inputs accepted by the [C API](api.md).
