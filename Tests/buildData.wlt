<|
  "$PostTagSystemRootDirectory" -> <|
    "tests" -> {
      VerificationTest[
        FileExistsQ @ $PostTagSystemRootDirectory
      ]
    }
  |>,

  "$PostTagSystemGitSHA" -> <|
    "tests" -> {
      VerificationTest[
        StringMatchQ[$PostTagSystemGitSHA, Repeated[HexadecimalCharacter, 40] ~~ Repeated["*", {0, 1}]]
      ]
    }
  |>,

  "$PostTagSystemBuildTime" -> <|
    "tests" -> {
      VerificationTest[
        DateObjectQ @ $PostTagSystemBuildTime
      ],

      VerificationTest[
        $PostTagSystemBuildTime["TimeZone"],
        "UTC"
      ],

      (* could not be built in the future *)
      VerificationTest[
        $PostTagSystemBuildTime < Now
      ],

      (* could not be built before $PostTagSystemBuildTime was implemented *)
      VerificationTest[
        DateObject[{2020, 3, 17, 0, 0, 0}, TimeZone -> "UTC"] < $PostTagSystemBuildTime
      ]
    }
  |>,

  "$PostTagSystemLibraryPath" -> <|
    "tests" -> {
      VerificationTest[
        StringQ @ $PostTagSystemLibraryPath
      ],

      VerificationTest[
        FileExistsQ @ $PostTagSystemLibraryPath
      ]
    }
  |>,

  "$PostTagSystemLibraryBuildTime" -> <|
    "tests" -> {
      VerificationTest[
        DateObjectQ @ $PostTagSystemLibraryBuildTime
      ],

      VerificationTest[
        $PostTagSystemLibraryBuildTime["TimeZone"],
        "UTC"
      ],

      (* could not be built in the future *)
      VerificationTest[
        $PostTagSystemLibraryBuildTime < Now
      ],

      (* could not be built before $PostTagSystemLibraryBuildTime was implemented *)
      VerificationTest[
        DateObject[{2020, 11, 22, 0, 0, 0}, TimeZone -> "UTC"] < $PostTagSystemLibraryBuildTime
      ]
    }
  |>
|>
