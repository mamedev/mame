.. _naming:

MAME Naming Conventions
=======================

.. contents:: :local:


.. _naming-intro:

Introduction
------------

To promote consistency and readability in MAME source code, we have some
naming conventions for various elements.


.. _naming-transliteration:

Transliteration
---------------

For better or worse, the most broadly recognised script in the world is
English Latin.  Conveniently, it’s also included in almost all character
encodings.  To make MAME more globally accessible, we require Latin
transliterations of titles and other metadata from other scripts.  Do
not use translations in metadata – translations are inherently
subjective and error-prone.  Translations may be included in comments if
they may be helpful.

If general, if an official Latin script name is known, it should be used
in favour of a naïve transliteration.  For titles containing foreign
loanwords, the conventional Latin spelling should be used for the
loanwords (the most obvious example of this is the use of “Mahjong” in
Japanese titles rather than “Maajan”).

Chinese
    Where the primary audience was Mandarin-speaking, Hanyu Pinyin
    should be used.  In contexts where diacritics are not permitted
    (e.g. when limited to ASCII), tone numbers should be omitted.  When
    tones are being indicated using diacritics, tone sandhi rules should
    be applied.  Where the primary audience was Cantonese-speaking
    (primarily Hong Kong and Guandong), Jyutping should be used with
    tone numbers omitted.  If in doubt, use Hanyu Pinyin.
Greek
    Use ISO 843:1997 type 2 (TR) rules.  Do not use traditional English
    spellings for Greek names (people or places).
Japanese
    Modified Hepburn rules should generally be used.  Use an apostrophe
    between syllabic N and a following vowel (including iotised vowels).
    Do not use hyphens to transliterate prolonged vowels.
Korean
    Use Revised Romanisation of Korean (RR) rules with traditional
    English spelling for Korean surnames.  Do not use ALA-LC rules for
    word division and use of hyphens.
Vietnamese
    When diacritics can’t be used, omit the tones and replace the vowels
    with single English vowels – do not use VIQR or TELEX conventions
    (“an chuot nuong” rather than “a(n chuo^.t nu*o*'ng” or “awn chuootj
    nuowngs”).


.. _naming-titles:

Titles and descriptions
-----------------------

Try to reproduce the original title faithfully where possible.  Try to
preserve the case convention used by the manufacturer/publisher.  If no
official English Latin title is known, use a standard transliteration.
For software list entries where a transliteration is used for the
``description`` element, put the title in an ``info`` element with a
``name="alt_title"`` attribute.

For software items that have multiple titles (for example different
regional titles with the same installation media), use the most most
widespread English Latin title for the ``description`` element, and put
the other titles in ``info`` elements with ``name="alt_title"``
attributes.

If disambiguation is needed, try to be descriptive as possible.  For
example, use the manufacturer’s version number, regional licensee’s
name, or terse description of hardware differences in preference to
arbitrary set numbers.  Surround the disambiguation text with
parentheses, preserve original case for names and version text, but
use lowercase for anything else besides proper nouns and initialisms.


.. _naming-cplusplus:

C++ naming conventions
----------------------

For C++ naming conventions, see the relevant section in the C++
Coding Guidelines: :ref:`contributing-cxx-naming`
