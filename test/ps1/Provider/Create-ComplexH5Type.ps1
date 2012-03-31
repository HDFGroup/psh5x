
$scope = 2
$path = [IO.Path]::GetTempFileName()
if (Test-Path $path)
{
    Remove-Item $path
}

$name = (Split-Path -Leaf $path).Split('.')[0]

New-PSDrive $name HDF5 "$($name):\" -Path $path -Mode RW -Force -Scope $scope 

cd "$($name):"

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
        "Base": "uchar",
        "Members": {
          "Red": 0,
          "Green": 1,
          "Blue": 2
        }
      }
    ],

    "Location": [37,
      {
        "Class": "Array",
        "Base": "double",
        "Dims": [3]
      }
    ],

    "Group": [61, { "Class": "Reference", "Kind": "Object" }],

    "Surveyed areas": [69, { "Class": "Reference", "Kind": "Region" } ]
  },

  "Size": 81 
}
"@

New-Item 'What a type!' -ItemType Datatype -Definition $t -Force

c:

Remove-PSDrive $name -Scope $scope 
