$t = @"
{
  "Class": "Compound",

  "Members": {

    "Sensors": [0,
      {
        "Class": "Vlen",
        "Base": {
          "Class": "Compound",
          "Members": {
            "Serial numbers": [0, "int"],
            "Location": [4, "string"],
            "Temperature (F)": [12, "double"],
            "Pressure (inHg)": [20, "H5T_IEEE_F64LE"]
          },
          "Size": 28
        }
      }
    ],

    "Name": [28, "string"],

    "Color": [36,
      {
        "Class": "Enum",
        "Members": {
          "Red": 0,
          "Green": 1,
          "Blue": 2
        }
      }
    ],

    "Location": [40,
      {
        "Class": "Array",
        "Base": "double",
        "Dims": [3]
      }
    ],

    "Group": [64, { "Class": "Reference", "Kind": "Object" }],

    "Surveyed areas": [72, { "Class": "Reference", "Kind": "Region" } ]
  },

  "Size": 84 
}
"@

New-H5LinkedDatatype 'h5tmp:\What a type!' $t
